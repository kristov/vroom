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
#include "vroom_pb.h"
#include "vrms_runtime.h"
#include "safemalloc.h"

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#define MAX_MSG_SIZE 1024

struct sock_ev_serv {
    ev_io io;
    int fd;
    struct sockaddr_un socket;
    int socket_len;
    array clients;
    vrms_runtime_t* vrms_runtime;
};

struct sock_ev_client {
    ev_io io;
    int fd;
    int index;
    struct sock_ev_serv* server;
    uint32_t vrms_scene_id;
};

uint32_t receive_create_scene(vrms_runtime_t* vrms_runtime, uint8_t* in_buf, int32_t length, uint32_t* error) {
    uint32_t id;
    CreateScene* cs_msg;

    if (!vrms_runtime) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    cs_msg = create_scene__unpack(NULL, length, in_buf);
    if (!cs_msg) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    id = vrms_runtime->interface->create_scene(vrms_runtime, cs_msg->name);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create scene: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_memory(vrms_runtime_t* vrms_runtime, uint8_t* in_buf, int32_t length, uint32_t* error, int shm_fd) {
    uint32_t id;
    CreateMemory* cs_msg;

    if (!vrms_runtime) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    cs_msg = create_memory__unpack(NULL, length, in_buf);
    if (!cs_msg) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    id = vrms_runtime->interface->create_memory(vrms_runtime, cs_msg->scene_id, shm_fd, cs_msg->size);
    if (0 == id) {
        fprintf(stderr, "create memory: out of memory\n");
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_data_object(vrms_runtime_t* vrms_runtime, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    uint32_t id;
    CreateDataObject* msg;

    if (!vrms_runtime) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    msg = create_data_object__unpack(NULL, length, in_buf);
    if (!msg) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    vrms_data_type_t vrms_type;
    switch (msg->type) {
        case CREATE_DATA_OBJECT__TYPE__UV:
            debug_print("received data type: VRMS_UV\n");
            vrms_type = VRMS_UV;
            break;
        case CREATE_DATA_OBJECT__TYPE__COLOR:
            debug_print("received data type: VRMS_COLOR\n");
            vrms_type = VRMS_COLOR;
            break;
        case CREATE_DATA_OBJECT__TYPE__VERTEX:
            debug_print("received data type: VRMS_VERTEX\n");
            vrms_type = VRMS_VERTEX;
            break;
        case CREATE_DATA_OBJECT__TYPE__NORMAL:
            debug_print("received data type: VRMS_NORMAL\n");
            vrms_type = VRMS_NORMAL;
            break;
        case CREATE_DATA_OBJECT__TYPE__INDEX:
            debug_print("received data type: VRMS_INDEX\n");
            vrms_type = VRMS_INDEX;
            break;
        case CREATE_DATA_OBJECT__TYPE__MATRIX:
            debug_print("received data type: VRMS_MATRIX\n");
            vrms_type = VRMS_MATRIX;
            break;
        case CREATE_DATA_OBJECT__TYPE__TEXTURE:
            debug_print("received data type: VRMS_TEXTURE\n");
            vrms_type = VRMS_TEXTURE;
            break;
        case CREATE_DATA_OBJECT__TYPE__PROGRAM:
            debug_print("received data type: VRMS_PROGRAM\n");
            vrms_type = VRMS_PROGRAM;
            break;
        case CREATE_DATA_OBJECT__TYPE__REGISTER:
            debug_print("received data type: VRMS_REGISTER\n");
            vrms_type = VRMS_REGISTER;
            break;
        case _CREATE_DATA_OBJECT__TYPE_IS_INT_SIZE:
            break;
    }

    id = vrms_runtime->interface->create_object_data(vrms_runtime, msg->scene_id, msg->memory_id, msg->memory_offset, msg->memory_length, msg->item_length, msg->data_length, vrms_type);
    if (0 == id) {
        fprintf(stderr, "create object data: out of memory\n");
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_create_texture_object(vrms_runtime_t* vrms_runtime, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    uint32_t id;
    CreateTextureObject* msg;

    if (!vrms_runtime) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    msg = create_texture_object__unpack(NULL, length, in_buf);
    if (!msg) {
        fprintf(stderr, "unpacking incoming message\n");
        *error = VRMS_INVALIDREQUEST;
        return 0;
    }

    vrms_texture_format_t format = msg->format;
    vrms_texture_type_t type = msg->type;

    id = vrms_runtime->interface->create_object_texture(vrms_runtime, msg->scene_id, msg->data_id, msg->width, msg->height, format, type);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create object texture: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_create_geometry_object(vrms_runtime_t* vrms_runtime, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    uint32_t id;
    CreateGeometryObject* cs_msg;

    if (!vrms_runtime) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    cs_msg = create_geometry_object__unpack(NULL, length, in_buf);
    if (!cs_msg) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    id = vrms_runtime->interface->create_object_geometry(vrms_runtime, cs_msg->scene_id, cs_msg->vertex_id, cs_msg->normal_id, cs_msg->index_id);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create object geometry: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_mesh_color(vrms_runtime_t* vrms_runtime, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    uint32_t id;
    CreateMeshColor* cs_msg;

    if (!vrms_runtime) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    cs_msg = create_mesh_color__unpack(NULL, length, in_buf);
    if (!cs_msg) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    id = vrms_runtime->interface->create_object_mesh_color(vrms_runtime, cs_msg->scene_id, cs_msg->geometry_id, cs_msg->r, cs_msg->g, cs_msg->b, cs_msg->a);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create object mesh color: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_mesh_texture(vrms_runtime_t* vrms_runtime, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    uint32_t id;
    CreateMeshTexture* cs_msg;

    if (!vrms_runtime) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    cs_msg = create_mesh_texture__unpack(NULL, length, in_buf);
    if (!cs_msg) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    id = vrms_runtime->interface->create_object_mesh_texture(vrms_runtime, cs_msg->scene_id, cs_msg->geometry_id, cs_msg->texture_id, cs_msg->uv_id);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create object mesh texture: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_program(vrms_runtime_t* vrms_runtime, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    uint32_t id;
    CreateProgram* msg;

    if (!vrms_runtime) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    msg = create_program__unpack(NULL, length, in_buf);
    if (!msg) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    id = vrms_runtime->interface->create_program(vrms_runtime, msg->scene_id, msg->data_id);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create program: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_run_program(vrms_runtime_t* vrms_runtime, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    uint32_t id;
    RunProgram* msg;

    if (!vrms_runtime) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    msg = run_program__unpack(NULL, length, in_buf);
    if (!msg) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    id = vrms_runtime->interface->run_program(vrms_runtime, msg->scene_id, msg->program_id, msg->register_id);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "run program: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_update_system_matrix(vrms_runtime_t* vrms_runtime, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    uint32_t ok;
    UpdateSystemMatrix* msg;
    vrms_matrix_type_t matrix_type;
    vrms_update_type_t update_type;

    if (!vrms_runtime) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    msg = update_system_matrix__unpack(NULL, length, in_buf);
    if (!msg) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    switch (msg->matrix_type) {
        case UPDATE_SYSTEM_MATRIX__MATRIX_TYPE__HEAD:
            matrix_type = VRMS_MATRIX_HEAD;
            break;
        case UPDATE_SYSTEM_MATRIX__MATRIX_TYPE__BODY:
            matrix_type = VRMS_MATRIX_BODY;
            break;
        case _UPDATE_SYSTEM_MATRIX__MATRIX_TYPE_IS_INT_SIZE:
            break;
    }

    switch (msg->update_type) {
        case UPDATE_SYSTEM_MATRIX__UPDATE_TYPE__MULTIPLY:
            update_type = VRMS_UPDATE_MULTIPLY;
            break;
        case UPDATE_SYSTEM_MATRIX__UPDATE_TYPE__SET:
            update_type = VRMS_UPDATE_SET;
            break;
        case _UPDATE_SYSTEM_MATRIX__UPDATE_TYPE_IS_INT_SIZE:
            break;
    }

    ok = vrms_runtime->interface->update_system_matrix(vrms_runtime, msg->scene_id, msg->data_id, msg->data_index, matrix_type, update_type);
    if (0 == ok) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "set system matrix error\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return ok;
}

uint32_t receive_create_skybox(vrms_runtime_t* vrms_runtime, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    uint32_t id;
    CreateSkybox* msg;

    if (!vrms_runtime) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    msg = create_skybox__unpack(NULL, length, in_buf);
    if (!msg) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    id = vrms_runtime->interface->create_object_skybox(vrms_runtime, msg->scene_id, msg->texture_id);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create skybox: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_destroy_object(vrms_runtime_t* vrms_runtime, uint8_t* in_buf, uint32_t length, uint32_t* error) {
    if (!vrms_runtime) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "vroom_protocol: server not initialized\n");
        return 0;
    }

    DestroyObject* msg = destroy_object__unpack(NULL, length, in_buf);
    if (!msg) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "vroom_protocol: unpacking incoming message\n");
        return 0;
    }

    uint8_t ok = vrms_runtime->interface->destroy_object(vrms_runtime, msg->scene_id, msg->id);
    if (0 == ok) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "vroom_protocol: destroy object some error\n");
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
    vrms_runtime_t* vrms_runtime;

    if (!client->server) {
        fprintf(stderr, "vroom_protocol: no server initialized\n");
        send_reply(client->fd, id, &error);
        return;
    }
    if (!client->server->vrms_runtime) {
        fprintf(stderr, "vroom_protocol: no VRMS server initialized\n");
        send_reply(client->fd, id, &error);
    }
    vrms_runtime = client->server->vrms_runtime;

    length_r = recv(client->fd, &type_c, 1, 0);
    if (length_r <= 0) {
        if (0 == length_r) {
            printf("orderly disconnect\n");
            if (client->vrms_scene_id > 0) {
                fprintf(stderr, "vroom_protocol: destroying scene: %d\n", client->vrms_scene_id);
                vrms_runtime->interface->destroy_scene(vrms_runtime, client->vrms_scene_id);
            }
            ev_io_stop(EV_A_ &client->io);
            close(client->fd);
            return;
        }
        else if (EAGAIN == errno) {
            fprintf(stderr, "vroom_protocol: should never get in this state with libev\n");
        }
        else {
            perror("recv\n");
        }
        return;
    }
    vrms_type_t type = (vrms_type_t)type_c;

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
        fprintf(stderr, "error receiving control fd\n");
        send_reply(client->fd, id, &error);
        return;
    }

    if (MAX_MSG_SIZE == length_r) {
        fprintf(stderr, "maximum message length exceeded\n");
        send_reply(client->fd, id, &error);
        return;
    }

    cmsgh = CMSG_FIRSTHDR(&msgh);
    if (!cmsgh) {
        fprintf(stderr, "expected one recvmsg header with an fd but got zero headers\n");
        send_reply(client->fd, id, &error);
        return;
    }

    if (cmsgh->cmsg_level != SOL_SOCKET) {
        fprintf(stderr, "invalid cmsg_level\n");
        send_reply(client->fd, id, &error);
        return;
    }

    if (cmsgh->cmsg_type != SCM_RIGHTS) {
        fprintf(stderr, "invalid cmsg_type\n");
        send_reply(client->fd, id, &error);
        return;
    }

    shm_fd = *((int *)CMSG_DATA(cmsgh));

    switch (type) {
        case VRMS_REPLY:
            error = VRMS_INVALIDREQUEST;
            fprintf(stderr, "received a reply message as a request\n");
            break;
        case VRMS_CREATESCENE:
            if (client->vrms_scene_id > 0) {
                error = VRMS_INVALIDREQUEST;
                fprintf(stderr, "connection already associated with a scene\n");
                id = 0;
            }
            else {
                id = receive_create_scene(vrms_runtime, in_buf, length_r, &error);
                fprintf(stderr, "scene: %d\n", id);
                if (id > 0) {
                    client->vrms_scene_id = id;
                }
            }
            break;
        case VRMS_DESTROYSCENE:
            break;
        case VRMS_CREATEMEMORY:
            id = receive_create_memory(vrms_runtime, in_buf, length_r, &error, shm_fd);
            break;
        case VRMS_CREATEDATAOBJECT:
            id = receive_create_data_object(vrms_runtime, in_buf, length_r, &error);
            break;
        case VRMS_CREATETEXTUREOBJECT:
            id = receive_create_texture_object(vrms_runtime, in_buf, length_r, &error);
            break;
        case VRMS_CREATEGEOMETRYOBJECT:
            id = receive_create_geometry_object(vrms_runtime, in_buf, length_r, &error);
            break;
        case VRMS_CREATEMESHCOLOR:
            id = receive_create_mesh_color(vrms_runtime, in_buf, length_r, &error);
            break;
        case VRMS_CREATEMESHTEXTURE:
            id = receive_create_mesh_texture(vrms_runtime, in_buf, length_r, &error);
            break;
        case VRMS_CREATEPROGRAM:
            id = receive_create_program(vrms_runtime, in_buf, length_r, &error);
            break;
        case VRMS_CREATESKYBOX:
            id = receive_create_skybox(vrms_runtime, in_buf, length_r, &error);
            break;
        case VRMS_DESTROYOBJECT:
            break;
        case VRMS_RUNPROGRAM:
            id = receive_run_program(vrms_runtime, in_buf, length_r, &error);
            break;
        case VRMS_UPDATESYSTEMMATRIX:
            id = receive_update_system_matrix(vrms_runtime, in_buf, length_r, &error);
            break;
        default:
            id = 0;
            error = VRMS_INVALIDREQUEST;
            fprintf(stderr, "unknown error\n");
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
    fprintf(stderr, "unix stream socket has become readable\n");

    int client_fd;
    struct sock_ev_client* client;
    struct sock_ev_serv* server = (struct sock_ev_serv*) w;

    while (1) {
        client_fd = accept(server->fd, NULL, NULL);
        if( client_fd == -1 ) {
            if( errno != EAGAIN && errno != EWOULDBLOCK ) {
                fprintf(stderr, "accept() failed errno=%i (%s)\n",  errno, strerror(errno));
                exit(1);
            }
            break;
        }
        printf("accepted a client\n");
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

    if (-1 == bind(server->fd, (struct sockaddr*) &server->socket, server->socket_len))
    {
      fprintf(stderr, "error server bind\n");
      exit(1);
    }

    if (-1 == listen(server->fd, max_queue)) {
      fprintf(stderr, "error listen\n");
      exit(1);
    }
    return 0;
}

void* run_module(vrms_runtime_t* vrms_runtime) {
    int max_queue = 128;
    struct sock_ev_serv server;
    EV_P = ev_default_loop(0);

    server_init(&server, "/tmp/libev-echo.sock", max_queue);
    server.vrms_runtime = vrms_runtime;

    ev_io_init(&server.io, server_cb, server.fd, EV_READ);
    ev_io_start(EV_A_ &server.io);

    ev_loop(EV_A_ 0);

    close(server.fd);
    return NULL;
}

