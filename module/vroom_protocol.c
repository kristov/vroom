#include <stdio.h>
#include <stdlib.h>
#include <ev.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include "array_heap.h"
#include "pb.h"
#include "runtime.h"
#include "safemalloc.h"
#include "vroom_protocol.h"

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#define MAX_MSG_SIZE 1024

struct sock_ev_serv {
    ev_io io;
    int fd;
    struct sockaddr_un socket;
    int socket_len;
    array clients;
    vrms_module_t* module;
};

struct sock_ev_client {
    ev_io io;
    int fd;
    int index;
    struct sock_ev_serv* server;
    uint32_t vrms_scene_id;
};

uint32_t receive_create_scene(vrms_module_t* module, uint8_t* in_buf, int32_t length, uint32_t* error) {
    uint32_t id;
    CreateScene* cs_msg;

    if (!module) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_create_scene(): server not initialized");
        return 0;
    }

    cs_msg = create_scene__unpack(NULL, length, in_buf);
    if (!cs_msg) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_create_scene(): error unpacking incoming message");
        return 0;
    }

    id = module->interface.create_scene(module, cs_msg->name);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        module->interface.error(module, "receive_create_scene(): out of memory");
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_memory(vrms_module_t* module, uint8_t* in_buf, int32_t length, uint32_t* error, int shm_fd) {
    uint32_t id;
    CreateMemory* cs_msg;

    if (!module) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_create_memory(): server not initialized");
        return 0;
    }

    cs_msg = create_memory__unpack(NULL, length, in_buf);
    if (!cs_msg) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_create_memory(): error unpacking incoming message");
        return 0;
    }

    id = module->interface.create_memory(module, cs_msg->scene_id, shm_fd, cs_msg->size);
    if (0 == id) {
        module->interface.error(module, "receive_create_memory(): out of memory");
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_data_object(vrms_module_t* module, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    uint32_t id;
    CreateDataObject* msg;

    if (!module) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_create_data_object(): server not initialized");
        return 0;
    }

    msg = create_data_object__unpack(NULL, length, in_buf);
    if (!msg) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_create_data_object(): unpacking incoming message");
        return 0;
    }

    vrms_data_type_t vrms_type;
    switch (msg->type) {
        case CREATE_DATA_OBJECT__TYPE__UINT8:
            debug_print("received data type: VRMS_UINT8\n");
            vrms_type = VRMS_UINT8;
            break;
        case CREATE_DATA_OBJECT__TYPE__UINT16:
            debug_print("received data type: VRMS_UINT16\n");
            vrms_type = VRMS_UINT16;
            break;
        case CREATE_DATA_OBJECT__TYPE__UINT32:
            debug_print("received data type: VRMS_UINT32\n");
            vrms_type = VRMS_UINT32;
            break;
        case CREATE_DATA_OBJECT__TYPE__FLOAT:
            debug_print("received data type: VRMS_FLOAT\n");
            vrms_type = VRMS_FLOAT;
            break;
        case CREATE_DATA_OBJECT__TYPE__VEC2:
            debug_print("received data type: VRMS_VEC2\n");
            vrms_type = VRMS_VEC2;
            break;
        case CREATE_DATA_OBJECT__TYPE__VEC3:
            debug_print("received data type: VRMS_VEC3\n");
            vrms_type = VRMS_VEC3;
            break;
        case CREATE_DATA_OBJECT__TYPE__VEC4:
            debug_print("received data type: VRMS_VEC4\n");
            vrms_type = VRMS_VEC4;
            break;
        case CREATE_DATA_OBJECT__TYPE__MAT2:
            debug_print("received data type: VRMS_MAT2\n");
            vrms_type = VRMS_MAT2;
            break;
        case CREATE_DATA_OBJECT__TYPE__MAT3:
            debug_print("received data type: VRMS_MAT3\n");
            vrms_type = VRMS_MAT3;
            break;
        case CREATE_DATA_OBJECT__TYPE__MAT4:
            debug_print("received data type: VRMS_MAT4\n");
            vrms_type = VRMS_MAT4;
            break;
        case _CREATE_DATA_OBJECT__TYPE_IS_INT_SIZE:
            break;
    }

    id = module->interface.create_object_data(module, msg->scene_id, msg->memory_id, msg->memory_offset, msg->memory_length, vrms_type);
    if (0 == id) {
        module->interface.error(module, "receive_create_data_object(): out of memory");
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_create_texture_object(vrms_module_t* module, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    uint32_t id;
    CreateTextureObject* msg;

    if (!module) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_create_texture_object(): server not initialized");
        return 0;
    }

    msg = create_texture_object__unpack(NULL, length, in_buf);
    if (!msg) {
        module->interface.error(module, "receive_create_texture_object(): error unpacking incoming message");
        *error = VRMS_INVALIDREQUEST;
        return 0;
    }

    vrms_texture_format_t format = msg->format;
    vrms_texture_type_t type = msg->type;

    id = module->interface.create_object_texture(module, msg->scene_id, msg->data_id, msg->width, msg->height, format, type);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        module->interface.error(module, "receive_create_texture_object(): out of memory");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_attach_memory(vrms_module_t* module, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    if (!module) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_attach_memory(): server not initialized");
        return 0;
    }

    AttachMemory* msg = attach_memory__unpack(NULL, length, in_buf);
    if (!msg) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_attach_memory(): error unpacking incoming message");
        return 0;
    }

    uint32_t id = module->interface.attach_memory(module, msg->scene_id, msg->data_id);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        module->interface.error(module, "receive_attach_memory(): out of memory");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_run_program(vrms_module_t* module, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    uint32_t id;
    RunProgram* msg;

    if (!module) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_run_program(): server not initialized");
        return 0;
    }

    msg = run_program__unpack(NULL, length, in_buf);
    if (!msg) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_run_program(): error unpacking incoming message");
        return 0;
    }

    id = module->interface.run_program(module, msg->scene_id, msg->program_id, msg->register_id);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        module->interface.error(module, "receive_run_program(): out of memory");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_set_skybox(vrms_module_t* module, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    uint32_t id;
    SetSkybox* msg;

    if (!module) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_set_skybox(): server not initialized");
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    msg = set_skybox__unpack(NULL, length, in_buf);
    if (!msg) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_set_skybox(): error unpacking incoming message");
        return 0;
    }

    id = module->interface.set_skybox(module, msg->scene_id, msg->texture_id);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        module->interface.error(module, "receive_set_skybox(): out of memory");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_destroy_object(vrms_module_t* module, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    if (!module) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_destroy_object(): server not initialized");
        return 0;
    }

    DestroyObject* msg = destroy_object__unpack(NULL, length, in_buf);
    if (!msg) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_destroy_object(): error unpacking incoming message");
        return 0;
    }

    uint8_t ok = module->interface.destroy_object(module, msg->scene_id, msg->id);
    if (0 == ok) {
        *error = VRMS_INVALIDREQUEST;
        module->interface.error(module, "receive_destroy_object(): destroy object some error");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return ok;
}

void send_reply(int32_t fd, int32_t id, uint32_t* error_code) {
    void *out_buf;
    size_t length;
    Reply re_msg = REPLY__INIT;

    re_msg.id = id;
    length = reply__get_packed_size(&re_msg);
    out_buf = SAFEMALLOC(length);

    re_msg.error_code = (int32_t)*error_code;

    reply__pack(&re_msg, out_buf);
    if (send(fd, out_buf, length, 0) < 0) {
        fprintf(stderr, "vroom_protocol: error sending reply to client\n");
    }

    free(out_buf);
}

static void client_cb(EV_P_ ev_io *w, int revents) {
    struct msghdr msgh;
    struct iovec iov;
    union {
        struct cmsghdr cmsgh;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;
    struct cmsghdr *cmsgh;

    struct sock_ev_client* client = (struct sock_ev_client*) w;
    uint32_t id = 0;
    int32_t shm_fd = 0;
    uint32_t error;
    uint8_t in_buf[MAX_MSG_SIZE];

    int32_t length_r;
    char type_c;
    vrms_module_t* module;

    if (!client->server) {
        fprintf(stderr, "vroom_protocol: no server initialized\n");
        send_reply(client->fd, id, &error);
        return;
    }
    if (!client->server->module) {
        fprintf(stderr, "vroom_protocol: no module initialized\n");
        send_reply(client->fd, id, &error);
    }
    module = client->server->module;

    length_r = recv(client->fd, &type_c, 1, 0);
    if (length_r <= 0) {
        if (0 == length_r) {
            printf("orderly disconnect\n");
            if (client->vrms_scene_id > 0) {
                module->interface.debug(module, "destroying scene: %d", client->vrms_scene_id);
                module->interface.destroy_scene(module, client->vrms_scene_id);
            }
            ev_io_stop(EV_A_ &client->io);
            close(client->fd);
            return;
        }
        else if (EAGAIN == errno) {
            module->interface.error(module, "should never get in this state with libev");
        }
        else {
            perror("recv\n");
        }
        return;
    }
    vroom_protocol_type_t type = (vroom_protocol_type_t)type_c;

    iov.iov_base = in_buf;
    iov.iov_len = MAX_MSG_SIZE;

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;
    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    msgh.msg_control = control_un.control;
    msgh.msg_controllen = sizeof(control_un.control);

    length_r = recvmsg(client->fd, &msgh, 0);
    if (-1 == length_r) {
        module->interface.error(module, "error receiving control fd");
        send_reply(client->fd, id, &error);
        return;
    }

    if (MAX_MSG_SIZE == length_r) {
        module->interface.error(module, "maximum message length exceeded");
        send_reply(client->fd, id, &error);
        return;
    }

    cmsgh = CMSG_FIRSTHDR(&msgh);
    if (!cmsgh) {
        module->interface.error(module, "expected one recvmsg header with an fd but got zero headers");
        send_reply(client->fd, id, &error);
        return;
    }

    if (cmsgh->cmsg_level != SOL_SOCKET) {
        module->interface.error(module, "invalid cmsg_level");
        send_reply(client->fd, id, &error);
        return;
    }

    if (cmsgh->cmsg_type != SCM_RIGHTS) {
        module->interface.error(module, "invalid cmsg_type");
        send_reply(client->fd, id, &error);
        return;
    }

    shm_fd = *((int *)CMSG_DATA(cmsgh));

    switch (type) {
        case VRMS_REPLY:
            error = VRMS_INVALIDREQUEST;
            module->interface.error(module, "received a reply message as a request");
            break;
        case VRMS_CREATESCENE:
            if (client->vrms_scene_id > 0) {
                error = VRMS_INVALIDREQUEST;
                module->interface.error(module, "connection already associated with a scene");
                id = 0;
            }
            else {
                id = receive_create_scene(module, in_buf, length_r, &error);
                module->interface.debug(module, "scene: %d", id);
                if (id > 0) {
                    client->vrms_scene_id = id;
                }
            }
            break;
        case VRMS_DESTROYSCENE:
            break;
        case VRMS_CREATEMEMORY:
            id = receive_create_memory(module, in_buf, length_r, &error, shm_fd);
            break;
        case VRMS_CREATEDATAOBJECT:
            id = receive_create_data_object(module, in_buf, length_r, &error);
            break;
        case VRMS_CREATETEXTUREOBJECT:
            id = receive_create_texture_object(module, in_buf, length_r, &error);
            break;
        case VRMS_DESTROYOBJECT:
            break;
        case VRMS_RUNPROGRAM:
            id = receive_run_program(module, in_buf, length_r, &error);
            break;
        case VRMS_SETSKYBOX:
            id = receive_set_skybox(module, in_buf, length_r, &error);
            break;
        default:
            id = 0;
            error = VRMS_INVALIDREQUEST;
            module->interface.error(module, "unknown reqeust type");
            break;
    }

    send_reply(client->fd, id, &error);
}

int setnonblock(int fd) {
    int flags;

    flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

inline static struct sock_ev_client* client_new(int fd) {
    struct sock_ev_client* client;

    client = realloc(NULL, sizeof(struct sock_ev_client));
    client->fd = fd;
    setnonblock(client->fd);
    ev_io_init(&client->io, client_cb, client->fd, EV_READ);

    return client;
}

static void server_cb(EV_P_ ev_io *w, int revents) {
    int client_fd;
    struct sock_ev_client* client;
    struct sock_ev_serv* server = (struct sock_ev_serv*) w;
    server->module->interface.debug(server->module, "unix stream socket has become readable");

    while (1) {
        client_fd = accept(server->fd, NULL, NULL);
        if( client_fd == -1 ) {
            if( errno != EAGAIN && errno != EWOULDBLOCK ) {
                server->module->interface.debug(server->module, "accept() failed errno=%i (%s)\n",  errno, strerror(errno));
                exit(1);
            }
            break;
        }
        server->module->interface.debug(server->module, "accepted a client");
        client = client_new(client_fd);
        client->vrms_scene_id = 0;
        client->server = server;
        client->index = array_push(&server->clients, client);
        ev_io_start(EV_A_ &client->io);
    }
}

int unix_socket_init(struct sockaddr_un* socket_un, char* sock_path, int max_queue) {
    int fd;

    unlink(sock_path);

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == fd) {
        fprintf(stderr, "error creating server socket\n");
        exit(1);
    }

    if (-1 == setnonblock(fd)) {
        fprintf(stderr, "error making server socket nonblocking\n");
        exit(1);
    }

    socket_un->sun_family = AF_UNIX;
    strcpy(socket_un->sun_path, sock_path);

    return fd;
}

int server_init(struct sock_ev_serv* server, char* sock_path, int max_queue) {
    server->fd = unix_socket_init(&server->socket, sock_path, max_queue);
    server->socket_len = sizeof(server->socket.sun_family) + strlen(server->socket.sun_path);

    array_init(&server->clients, 128);

    if (-1 == bind(server->fd, (struct sockaddr*) &server->socket, server->socket_len)) {
        server->module->interface.error(server->module, "error server bind");
        exit(1);
    }

    if (-1 == listen(server->fd, max_queue)) {
        server->module->interface.error(server->module, "error listen");
        exit(1);
    }
    return 0;
}

void* run_module(vrms_module_t* module) {
    int max_queue = 128;
    struct sock_ev_serv server;
    EV_P = ev_default_loop(0);

    server_init(&server, "/tmp/libev-echo.sock", max_queue);
    server.module = module;

    ev_io_init(&server.io, server_cb, server.fd, EV_READ);
    ev_io_start(EV_A_ &server.io);

    module->interface.debug(module, "initialized");
    ev_loop(EV_A_ 0);

    close(server.fd);
    return NULL;
}

