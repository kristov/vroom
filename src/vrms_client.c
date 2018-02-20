#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syscall.h>
#include <sys/un.h>
#include <linux/memfd.h>
#include <unistd.h>
#include "memfd.h"
#include "safe_malloc.h"
#include "vroom_pb.h"
#include "vrms_client.h"

#define SOCK_PATH "/tmp/libev-echo.sock"
#define MAX_MSG_SIZE 1024

uint32_t data_object_type_map[] = {
    CREATE_DATA_OBJECT__TYPE__UV,       // VRMS_UV
    CREATE_DATA_OBJECT__TYPE__COLOR,    // VRMS_COLOR
    CREATE_DATA_OBJECT__TYPE__VERTEX,   // VRMS_VERTEX
    CREATE_DATA_OBJECT__TYPE__NORMAL,   // VRMS_NORMAL
    CREATE_DATA_OBJECT__TYPE__INDEX,    // VRMS_INDEX
    CREATE_DATA_OBJECT__TYPE__MATRIX    // VRMS_MATRIX
};

uint32_t matrix_type_map[] = {
    UPDATE_SYSTEM_MATRIX__MATRIX_TYPE__HEAD,
    UPDATE_SYSTEM_MATRIX__MATRIX_TYPE__BODY
};

uint32_t update_type_map[] = {
    UPDATE_SYSTEM_MATRIX__UPDATE_TYPE__MULTIPLY,
    UPDATE_SYSTEM_MATRIX__UPDATE_TYPE__SET
};

uint32_t format_map[] = {
    CREATE_TEXTURE_OBJECT__FORMAT__RGB8  // VRMS_RGB8
};

int32_t destroy_shared_memory(int32_t fd) {
    close(fd);
    return 0;
}

uint32_t vrms_client_receive_reply(vrms_client_t* client) {
    int32_t id = 0;
    size_t count_recv;
    uint8_t in_buf[MAX_MSG_SIZE];
    Reply* re_msg;

    count_recv = recv(client->socket, in_buf, MAX_MSG_SIZE, 0);
    if (count_recv <= 0) {
        if (0 == count_recv) {
            fprintf(stderr, "orderly disconnect\n");
        }
        else {
            fprintf(stderr, "recv error: %zu\n", count_recv);
        }
        return 0;
    }

    re_msg = reply__unpack(NULL, count_recv, in_buf);   
    if (NULL == re_msg) {
        fprintf(stderr, "error unpacking incoming message from length: %zu\n", count_recv);
        return 0;
    }

    if (re_msg->error_code > 0) {
        fprintf(stderr, "error: %d\n", re_msg->error_code);
    }
    else {
        if (re_msg->id > 0) {
            id = re_msg->id;
        }
    }

    reply__free_unpacked(re_msg, NULL);

    return id;
}

uint32_t vrms_client_send_message(vrms_client_t* client, vrms_type_t type, void* buffer, uint32_t length, int32_t fd) {
    struct msghdr msgh;
    struct iovec iov;
    union {
        struct cmsghdr cmsgh;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;

    char type_c = (char)type;

    if (send(client->socket, &type_c, 1, 0) == -1) {
        return 0;
    }

    if (fd == -1) {
        fprintf(stderr, "Cannot pass an invalid fd equaling -1\n");
        return 0;
    }

    iov.iov_base = buffer;
    iov.iov_len = length;

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;
    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    msgh.msg_control = control_un.control;
    msgh.msg_controllen = sizeof(control_un.control);

    control_un.cmsgh.cmsg_len = CMSG_LEN(sizeof(int));
    control_un.cmsgh.cmsg_level = SOL_SOCKET;
    control_un.cmsgh.cmsg_type = SCM_RIGHTS;
    *((int *)CMSG_DATA(CMSG_FIRSTHDR(&msgh))) = fd;

    int size = sendmsg(client->socket, &msgh, 0);
    if (size < 0) {
        fprintf(stderr, "Error sending fd\n");
        return 0;
    }

    return vrms_client_receive_reply(client);
}

uint32_t vrms_client_create_scene(vrms_client_t* client, char* name) {
    uint32_t id;
    CreateScene msg = CREATE_SCENE__INIT;
    void* buf;
    uint32_t length;

    msg.name = name;
    length = create_scene__get_packed_size(&msg);
  
    buf = SAFEMALLOC(length);
    create_scene__pack(&msg, buf);

    id = vrms_client_send_message(client, VRMS_CREATESCENE, buf, length, 0);
  
    free(buf);
    return id;
}

uint32_t vrms_client_create_memory(vrms_client_t* client, uint8_t** address, size_t size) {
    int32_t fd = -1;
    int32_t ret;
    uint32_t id;
    void* buf;
    uint32_t length;

    fd = memfd_create("VROOM memfd", MFD_ALLOW_SEALING);
    if (fd <= 0) {
        fprintf(stderr, "unable to create shared memory: %d\n", errno);
        return -1;
    }

    ret = ftruncate(fd, size);
    if (-1 == ret) {
        fprintf(stderr, "unable to truncate memfd to size %zd\n", size);
        return -1;
    }

    ret = fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK);
    if (-1 == ret) {
        fprintf(stderr, "failed to add seals to memfd\n");
        return -1;
    }

    *address = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (MAP_FAILED == *address) {
        fprintf(stderr, "unable to attach address\n");
        return -1;
    }

    CreateMemory msg = CREATE_MEMORY__INIT;
    msg.scene_id = client->scene_id;
    msg.size = size;

    length = create_memory__get_packed_size(&msg);

    buf = SAFEMALLOC(length);
    create_memory__pack(&msg, buf);

    id = vrms_client_send_message(client, VRMS_CREATEMEMORY, buf, length, fd);

    free(buf);
    return id;
}

uint32_t vrms_client_create_data_object(vrms_client_t* client, vrms_data_type_t type, int32_t memory_id, uint32_t offset, uint32_t size, uint32_t nr_strides, uint32_t stride) {
    uint32_t id;
    CreateDataObject msg = CREATE_DATA_OBJECT__INIT;
    void* buf;
    uint32_t length;

    uint32_t data_object_type_map_index = (uint32_t)type;
    if (data_object_type_map_index < 0 || data_object_type_map_index > 6) {
        return 0;
    }
    uint32_t pb_type = data_object_type_map[data_object_type_map_index];

    msg.scene_id = client->scene_id;
    msg.memory_id = memory_id;
    msg.type = pb_type;
    msg.offset = offset;
    msg.size = size;
    msg.nr_strides = nr_strides;
    msg.stride = stride;

    length = create_data_object__get_packed_size(&msg);

    buf = SAFEMALLOC(length);
    create_data_object__pack(&msg, buf);

    id = vrms_client_send_message(client, VRMS_CREATEDATAOBJECT, buf, length, 0);

    free(buf);
    return id;
}

uint32_t vrms_client_create_texture_object(vrms_client_t* client, int32_t memory_id, uint32_t offset, uint32_t size, uint32_t width, uint32_t height, vrms_texture_format_t format) {
    uint32_t id;
    CreateTextureObject msg = CREATE_TEXTURE_OBJECT__INIT;
    void* buf;
    uint32_t length;

    uint32_t format_map_index = (uint32_t)format;
    if (format_map_index < 0 || format_map_index > 0) {
        return 0;
    }
    uint32_t pb_format = format_map[format_map_index];

    msg.scene_id = client->scene_id;
    msg.memory_id = memory_id;
    msg.offset = offset;
    msg.size = size;
    msg.width = width;
    msg.height = height;
    msg.format = pb_format;

    length = create_texture_object__get_packed_size(&msg);

    buf = SAFEMALLOC(length);
    create_texture_object__pack(&msg, buf);

    id = vrms_client_send_message(client, VRMS_CREATETEXTUREOBJECT, buf, length, 0);

    free(buf);
    return id;
}

uint32_t vrms_client_create_geometry_object(vrms_client_t* client, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id) {
    uint32_t id;
    CreateGeometryObject msg = CREATE_GEOMETRY_OBJECT__INIT;
    void* buf;
    uint32_t length;

    msg.scene_id = client->scene_id;
    msg.vertex_id = vertex_id;
    msg.normal_id = normal_id;
    msg.index_id = index_id;

    length = create_geometry_object__get_packed_size(&msg);
  
    buf = SAFEMALLOC(length);
    create_geometry_object__pack(&msg, buf);

    id = vrms_client_send_message(client, VRMS_CREATEGEOMETRYOBJECT, buf, length, 0);
  
    free(buf);
    return id;
}

uint32_t vrms_client_create_mesh_color(vrms_client_t* client, uint32_t geometry_id, float r, float g, float b, float a) {
    uint32_t id;
    CreateMeshColor msg = CREATE_MESH_COLOR__INIT;
    void* buf;
    uint32_t length;

    msg.scene_id = client->scene_id;
    msg.geometry_id = geometry_id;
    msg.r = r;
    msg.g = g;
    msg.b = b;
    msg.a = a;

    length = create_mesh_color__get_packed_size(&msg);
  
    buf = SAFEMALLOC(length);
    create_mesh_color__pack(&msg, buf);

    id = vrms_client_send_message(client, VRMS_CREATEMESHCOLOR, buf, length, 0);
  
    free(buf);
    return id;
}

uint32_t vrms_client_create_mesh_texture(vrms_client_t* client, uint32_t geometry_id, uint32_t texture_id, uint32_t uv_id) {
    uint32_t id;
    CreateMeshTexture msg = CREATE_MESH_TEXTURE__INIT;
    void* buf;
    uint32_t length;

    msg.scene_id = client->scene_id;
    msg.geometry_id = geometry_id;
    msg.texture_id = texture_id;
    msg.uv_id = uv_id;

    length = create_mesh_texture__get_packed_size(&msg);
  
    buf = SAFEMALLOC(length);
    create_mesh_texture__pack(&msg, buf);

    id = vrms_client_send_message(client, VRMS_CREATEMESHTEXTURE, buf, length, 0);
  
    free(buf);
    return id;
}

uint32_t vrms_client_render_buffer_set(vrms_client_t* client, int32_t memory_id, uint32_t nr_items) {
    uint32_t ret;
    SetRenderBuffer msg = SET_RENDER_BUFFER__INIT;
    void* buf;
    uint32_t length;

    msg.scene_id = client->scene_id;
    msg.memory_id = memory_id;
    msg.nr_objects = nr_items;

    length = set_render_buffer__get_packed_size(&msg);

    buf = SAFEMALLOC(length);
    set_render_buffer__pack(&msg, buf);

    ret = vrms_client_send_message(client, VRMS_SETRENDERBUFFER, buf, length, 0);

    free(buf);
    return ret;
}

uint32_t vrms_client_update_system_matrix(vrms_client_t* client, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, int32_t memory_id, uint32_t offset, uint32_t size) {
    uint32_t ret;
    UpdateSystemMatrix msg = UPDATE_SYSTEM_MATRIX__INIT;
    void* buf;
    uint32_t length;

    uint32_t matrix_type_index = (uint32_t)matrix_type;
    if (matrix_type_index < 0 || matrix_type_index > 1) {
        return 0;
    }
    uint32_t pb_matrix_type = matrix_type_map[matrix_type_index];

    uint32_t update_type_index = (uint32_t)update_type;
    if (update_type_index < 0 || update_type_index > 1) {
        return 0;
    }
    uint32_t pb_update_type = update_type_map[update_type_index];

    msg.scene_id = client->scene_id;
    msg.memory_id = memory_id;
    msg.matrix_type = pb_matrix_type;
    msg.update_type = pb_update_type;
    msg.offset = offset;
    msg.size = size;

    length = update_system_matrix__get_packed_size(&msg);

    buf = SAFEMALLOC(length);
    update_system_matrix__pack(&msg, buf);

    ret = vrms_client_send_message(client, VRMS_UPDATESYSTEMMATRIX, buf, length, 0);

    free(buf);
    return ret;
}

int32_t vrms_client_connect_socket(vrms_client_t* client) {
    int socket_name_length;
    struct sockaddr_un remote;

    if ((client->socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        return 1;
    }

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    socket_name_length = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(client->socket, (struct sockaddr *)&remote, socket_name_length) == -1) {
        return 1;
    }

    return 0;
}

vrms_client_t* vrms_connect() {
    vrms_client_t* client = SAFEMALLOC(sizeof(vrms_client_t));
    if (vrms_client_connect_socket(client)) {
        return NULL;
    }
    return client;
}

uint32_t vrms_create_scene(vrms_client_t* client, char* name) {
    uint32_t scene_id;
    scene_id = vrms_client_create_scene(client, name);
    if (0 == scene_id) {
        return 0;
    }
    client->scene_id = scene_id;
    return scene_id;
}

uint32_t vrms_destroy_scene(vrms_client_t* client) {
    close(client->socket);
    free(client);
    return 0;
}
