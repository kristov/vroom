#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "vroom.pb-c.h"
#include "vrms_client.h"

#define SOCK_PATH "/tmp/libev-echo.sock"
#define MMAP_PATH "/vrms_client"

#define MAX_MSG_SIZE 1024

uint32_t type_map[] = {
    CREATE_DATA_OBJECT__TYPE__UV,       // VRMS_UV
    CREATE_DATA_OBJECT__TYPE__COLOR,    // VRMS_COLOR
    CREATE_DATA_OBJECT__TYPE__TEXTURE,  // VRMS_TEXTURE
    CREATE_DATA_OBJECT__TYPE__VERTEX,   // VRMS_VERTEX
    CREATE_DATA_OBJECT__TYPE__NORMAL,   // VRMS_NORMAL
    CREATE_DATA_OBJECT__TYPE__INDEX     // VRMS_INDEX
};

uint32_t vrms_client_receive_reply(vrms_client_t* client) {
    int32_t id = 0;
    int count_recv;
    uint8_t in_buf[MAX_MSG_SIZE];
    Reply* re_msg;

    count_recv = recv(client->socket, in_buf, MAX_MSG_SIZE, 0);
    if (count_recv <= 0) {
        if (0 == count_recv) {
            fprintf(stderr, "orderly disconnect\n");
        }
        else {
            fprintf(stderr, "recv\n");
        }
        return 0;
    }

    re_msg = reply__unpack(NULL, count_recv, in_buf);   
    if (re_msg == NULL) {
        printf("error unpacking incoming message\n");
        return 0;
    }

    if (re_msg->error_code > 0) {
        printf("error: %d\n", re_msg->error_code);
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
  
    buf = malloc(length);
    create_scene__pack(&msg, buf);

    id = vrms_client_send_message(client, VRMS_CREATESCENE, buf, length, 0);
  
    free(buf);
    return id;
}

uint32_t vrms_client_create_data_object(vrms_client_t* client, vrms_data_type_t type, int32_t shm_fd, uint32_t offset, uint32_t size_of, uint32_t stride) {
    uint32_t id;
    CreateDataObject msg = CREATE_DATA_OBJECT__INIT;
    void* buf;
    uint32_t length;

    uint32_t type_map_index = (uint32_t)type;
    if (type_map_index < 0 || type_map_index > 5) {
        return 0;
    }
    uint32_t pb_type = type_map[type_map_index];

    msg.scene_id = client->scene_id;
    msg.type = pb_type;
    msg.shm_fd = shm_fd;
    msg.offset = offset;
    msg.size_of = size_of;
    msg.stride = stride;

    length = create_data_object__get_packed_size(&msg);
  
    buf = malloc(length);
    create_data_object__pack(&msg, buf);

    id = vrms_client_send_message(client, VRMS_CREATEDATAOBJECT, buf, length, shm_fd);
  
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
  
    buf = malloc(length);
    create_geometry_object__pack(&msg, buf);

    id = vrms_client_send_message(client, VRMS_CREATEGEOMETRYOBJECT, buf, length, 0);
  
    free(buf);
    return id;
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
    vrms_client_t* client = malloc(sizeof(vrms_client_t));
    if (vrms_client_connect_socket(client)) {
        return NULL;
    }
    return client;
}

uint32_t vrms_create_scene(vrms_client_t* client, char* name) {
    uint32_t scene_id;
    scene_id = vrms_client_create_scene(client, name);
    if (0 == scene_id) {
        return 1;
    }

    return 0;
}

uint32_t vrms_destroy_scene(vrms_client_t* client) {
    close(client->socket);
    free(client);
    return 0;
}

int32_t vrms_create_memory(size_t size, void** address) {
    int32_t shm_fd = -1; 

    shm_fd = shm_open(MMAP_PATH, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    if (-1 == shm_fd) {
        fprintf(stderr, "unable to create shared memory: %d\n", errno);
        return -1;
    }
    ftruncate(shm_fd, size);

    *address = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (MAP_FAILED == *address) {
        fprintf(stderr, "unable to attach address\n");
        shm_unlink(MMAP_PATH);
        return -1;
    }

    return shm_fd;
}

int32_t destroy_shared_memory(int32_t shm_fd) {
    shm_unlink(MMAP_PATH);
    return 0;
}

