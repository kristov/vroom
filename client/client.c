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
#include "safemalloc.h"
#include "pb.h"
#include "client.h"

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#define SOCK_PATH "/tmp/libev-echo.sock"
#define MAX_MSG_SIZE 1024

vrms_client_interface_t client_interface;

void fill_interface() {
    client_interface.create_scene = vrms_client_create_scene;
    client_interface.create_memory = vrms_client_create_memory;
    client_interface.create_object_data = vrms_client_create_object_data;
    client_interface.create_object_texture = vrms_client_create_object_texture;
    client_interface.run_program = vrms_client_run_program;
    client_interface.set_skybox = vrms_client_set_skybox;
    client_interface.destroy_scene = vrms_client_destroy_scene;
    //client_interface.destroy_object = vrms_client_destroy_object;
}

uint32_t data_object_type_map[] = {
    CREATE_DATA_OBJECT__TYPE__UINT8,
    CREATE_DATA_OBJECT__TYPE__UINT16,
    CREATE_DATA_OBJECT__TYPE__UINT32,
    CREATE_DATA_OBJECT__TYPE__FLOAT,
    CREATE_DATA_OBJECT__TYPE__VEC2,
    CREATE_DATA_OBJECT__TYPE__VEC3,
    CREATE_DATA_OBJECT__TYPE__VEC4,
    CREATE_DATA_OBJECT__TYPE__MAT2,
    CREATE_DATA_OBJECT__TYPE__MAT3,
    CREATE_DATA_OBJECT__TYPE__MAT4
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
    CREATE_TEXTURE_OBJECT__FORMAT__BGR888,
    CREATE_TEXTURE_OBJECT__FORMAT__XBGR8888,
    CREATE_TEXTURE_OBJECT__FORMAT__ABGR8888,
    CREATE_TEXTURE_OBJECT__FORMAT__RGB888,
    CREATE_TEXTURE_OBJECT__FORMAT__XRGB8888,
    CREATE_TEXTURE_OBJECT__FORMAT__ARGB8888
};

uint32_t texture_type_map[] = {
    CREATE_TEXTURE_OBJECT__TYPE__TEXTURE_2D,       // VRMS_TEXTURE_2D
    CREATE_TEXTURE_OBJECT__TYPE__TEXTURE_CUBE_MAP  // VRMS_TEXTURE_CUBE_MAP
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
    if (!re_msg) {
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

uint32_t vrms_client_create_scene_msg(vrms_client_t* client, char* name) {
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

uint32_t vrms_client_create_memory(vrms_client_t* client, int32_t fd, uint32_t size) {
    CreateMemory msg = CREATE_MEMORY__INIT;
    msg.scene_id = client->scene_id;
    msg.size = size;

    uint32_t length = create_memory__get_packed_size(&msg);

    void* buf = SAFEMALLOC(length);
    create_memory__pack(&msg, buf);

    uint32_t id = vrms_client_send_message(client, VRMS_CREATEMEMORY, buf, length, fd);

    free(buf);
    return id;
}

uint32_t vrms_client_create_object_data(vrms_client_t* client, uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, vrms_data_type_t type) {
    CreateDataObject msg = CREATE_DATA_OBJECT__INIT;

    uint32_t data_object_type_map_index = (uint32_t)type;
    if (data_object_type_map_index < 0 || data_object_type_map_index > 8) {
        return 0;
    }
    uint32_t pb_type = data_object_type_map[data_object_type_map_index];

    msg.scene_id = client->scene_id;
    msg.memory_id = memory_id;
    msg.memory_offset = memory_offset;
    msg.memory_length = memory_length;
    msg.type = pb_type;

    uint32_t length = create_data_object__get_packed_size(&msg);

    void* buf = SAFEMALLOC(length);
    create_data_object__pack(&msg, buf);

    uint32_t id = vrms_client_send_message(client, VRMS_CREATEDATAOBJECT, buf, length, 0);

    free(buf);
    return id;
}

uint32_t vrms_client_create_object_texture(vrms_client_t* client, uint32_t data_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type) {
    uint32_t id;
    CreateTextureObject msg = CREATE_TEXTURE_OBJECT__INIT;
    void* buf;
    uint32_t length;

    uint32_t format_map_index = (uint32_t)format;
    if (format_map_index < 0 || format_map_index > 5) {
        return 0;
    }
    uint32_t pb_format = format_map[format_map_index];

    uint32_t texture_type_index = (uint32_t)type;
    if (texture_type_index < 0 || texture_type_index > 1) {
        return 0;
    }
    uint32_t pb_type = texture_type_map[texture_type_index];

    msg.scene_id = client->scene_id;
    msg.data_id = data_id;
    msg.width = width;
    msg.height = height;
    msg.format = pb_format;
    msg.type = pb_type;

    length = create_texture_object__get_packed_size(&msg);

    buf = SAFEMALLOC(length);
    create_texture_object__pack(&msg, buf);

    id = vrms_client_send_message(client, VRMS_CREATETEXTUREOBJECT, buf, length, 0);

    free(buf);
    return id;
}

uint32_t vrms_client_run_program(vrms_client_t* client, uint32_t program_id, uint32_t register_id) {
    uint32_t ret;
    RunProgram msg = RUN_PROGRAM__INIT;
    void* buf;
    uint32_t length;

    msg.scene_id = client->scene_id;
    msg.program_id = program_id;
    msg.register_id = register_id;

    length = run_program__get_packed_size(&msg);

    buf = SAFEMALLOC(length);
    run_program__pack(&msg, buf);

    ret = vrms_client_send_message(client, VRMS_RUNPROGRAM, buf, length, 0);

    free(buf);
    return ret;
}

uint32_t vrms_client_set_skybox(vrms_client_t* client, uint32_t texture_id) {
    uint32_t ret;
    SetSkybox msg = SET_SKYBOX__INIT;
    void* buf;
    uint32_t length;

    msg.scene_id = client->scene_id;
    msg.texture_id = texture_id;

    length = set_skybox__get_packed_size(&msg);

    buf = SAFEMALLOC(length);
    set_skybox__pack(&msg, buf);

    ret = vrms_client_send_message(client, VRMS_SETSKYBOX, buf, length, 0);

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

    fill_interface();
    client->interface = &client_interface;

    if (vrms_client_connect_socket(client)) {
        return NULL;
    }
    return client;
}

uint32_t vrms_client_create_scene(vrms_client_t* client, char* name) {
    uint32_t scene_id;
    scene_id = vrms_client_create_scene_msg(client, name);
    if (0 == scene_id) {
        return 0;
    }
    client->scene_id = scene_id;
    return scene_id;
}

uint32_t vrms_client_destroy_scene(vrms_client_t* client) {
    close(client->socket);
    free(client);
    return 0;
}
