#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ev.h>
#define g_warning printf
#include <unistd.h>
#include "array-heap.h"
#include "vroom.pb-c.h"
#include "vrms_server.h"

#define MAX_MSG_SIZE 1024

struct sock_ev_serv {
    ev_io io;
    int fd;
    struct sockaddr_un socket;
    int socket_len;
    array clients;
};

struct sock_ev_client {
    ev_io io;
    int fd;
    int index;
    struct sock_ev_serv* server;
};

int setnonblock(int fd);
static void not_blocked(EV_P_ ev_periodic *w, int revents);

uint32_t receive_create_scene(uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateScene* cs_msg;
    cs_msg = create_scene__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        printf("error unpacking incoming message\n");
        *error = VRMS_INVALIDREQUEST;
        return 0;
    }

    id = vrms_create_scene(cs_msg->name);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_data_object(uint8_t* in_buf, int32_t length, vrms_error_t* error, int shm_fd) {
    uint32_t id;
    CreateDataObject* cs_msg;
    cs_msg = create_data_object__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        printf("error unpacking incoming message\n");
        *error = VRMS_INVALIDREQUEST;
        return 0;
    }

    vrms_data_type_t vrms_type;
    switch (cs_msg->type) {
        case CREATE_DATA_OBJECT__TYPE__UV:
            vrms_type = VRMS_UV;
        break;
        case CREATE_DATA_OBJECT__TYPE__COLOR:
            vrms_type = VRMS_COLOR;
        break;
        case CREATE_DATA_OBJECT__TYPE__TEXTURE:
            vrms_type = VRMS_TEXTURE;
        break;
        case CREATE_DATA_OBJECT__TYPE__VERTEX:
            vrms_type = VRMS_VERTEX;
        break;
        case CREATE_DATA_OBJECT__TYPE__NORMAL:
            vrms_type = VRMS_NORMAL;
        break;
        case CREATE_DATA_OBJECT__TYPE__INDEX:
            vrms_type = VRMS_INDEX;
        break;
        case _CREATE_DATA_OBJECT__TYPE_IS_INT_SIZE:
        break;
    }

    id = vrms_create_data_object(cs_msg->scene_id, vrms_type, shm_fd, cs_msg->offset, cs_msg->size_of, cs_msg->stride);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_geometry_object(uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateGeometryObject* cs_msg;
    cs_msg = create_geometry_object__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        printf("error unpacking incoming message\n");
        *error = VRMS_INVALIDREQUEST;
        return 0;
    }

    id = vrms_create_geometry_object(cs_msg->scene_id, cs_msg->vertex_id, cs_msg->normal_id, cs_msg->index_id);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_color_mesh(uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateColorMesh* cs_msg;
    cs_msg = create_color_mesh__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        printf("error unpacking incoming message\n");
        *error = VRMS_INVALIDREQUEST;
        return 0;
    }

    id = vrms_create_color_mesh(cs_msg->scene_id, cs_msg->geometry_id, cs_msg->r, cs_msg->g, cs_msg->b, cs_msg->a);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_texture_mesh(uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateTextureMesh* cs_msg;
    cs_msg = create_texture_mesh__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        printf("error unpacking incoming message\n");
        *error = VRMS_INVALIDREQUEST;
        return 0;
    }

    id = vrms_create_texture_mesh(cs_msg->scene_id, cs_msg->geometry_id, cs_msg->uv_id, cs_msg->texture_id);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

void send_reply(int32_t fd, int32_t id, int32_t error) {
    void *out_buf;
    int32_t length;
    Reply re_msg = REPLY__INIT;

    re_msg.id = 1;
    length = reply__get_packed_size(&re_msg);
    out_buf = malloc(length);

    re_msg.error_code = (int32_t)error;
    reply__pack(&re_msg, out_buf);
    if (send(fd, out_buf, length, 0) < 0) {
        fprintf(stderr, "error sending reply to client\n");
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
    int32_t id = 0;
    int32_t shm_fd = 0;
    vrms_error_t error;
    uint8_t in_buf[MAX_MSG_SIZE];

    int32_t length_r;
    char type_c;

    length_r = recv(client->fd, &type_c, 1, 0);
    if (length_r <= 0) {
        if (0 == length_r) {
            printf("orderly disconnect\n");
            ev_io_stop(EV_A_ &client->io);
            close(client->fd);
        }
        else if (EAGAIN == errno) {
            printf("should never get in this state with libev\n");
        }
        else {
            perror("recv");
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
        fprintf(stderr, "Error receiving control fd\n");
        return;
    }

    if (MAX_MSG_SIZE == length_r) {
        fprintf(stderr, "Expected a placeholder message data of length 1\n");
        fprintf(stderr, "Received a message of length %d instead\n", length_r);
        return;
    }

    cmsgh = CMSG_FIRSTHDR(&msgh);
    if (!cmsgh) {
        fprintf(stderr, "Expected one recvmsg() header with a passed memfd fd. Got zero headers!\n");
        return;
    }

    if (cmsgh->cmsg_level != SOL_SOCKET) {
        fprintf(stderr, "invalid cmsg_level %d", cmsgh->cmsg_level);
    }

    if (cmsgh->cmsg_type != SCM_RIGHTS) {
        fprintf(stderr, "invalid cmsg_type %d", cmsgh->cmsg_type);
    }

    shm_fd = *((int *)CMSG_DATA(cmsgh));

    switch (type) {
        case VRMS_REPLY:
            error = VRMS_INVALIDREQUEST;
        break;
        case VRMS_CREATESCENE:
            id = receive_create_scene(in_buf, length_r, &error);
        break;
        case VRMS_DESTROYSCENE:
        break;
        case VRMS_CREATEDATAOBJECT:
            id = receive_create_data_object(in_buf, length_r, &error, shm_fd);
        break;
        case VRMS_DESTROYDATAOBJECT:
        break;
        case VRMS_CREATEGEOMETRYOBJECT:
            id = receive_create_geometry_object(in_buf, length_r, &error);
        break;
        case VRMS_CREATECOLORMESH:
            id = receive_create_color_mesh(in_buf, length_r, &error);
        break;
        case VRMS_CREATETEXTUREMESH:
            id = receive_create_texture_mesh(in_buf, length_r, &error);
        break;
        default:
            id = 0;
            error = VRMS_INVALIDREQUEST;
        break;
    }

    send_reply(client->fd, id, error);
}

inline static struct sock_ev_client* client_new(int fd) {
    struct sock_ev_client* client;

    client = realloc(NULL, sizeof(struct sock_ev_client));
    client->fd = fd;
    //client->server = server;
    setnonblock(client->fd);
    ev_io_init(&client->io, client_cb, client->fd, EV_READ);

    return client;
}

// This callback is called when data is readable on the unix socket.
static void server_cb(EV_P_ ev_io *w, int revents) {
    puts("unix stream socket has become readable");

    int client_fd;
    struct sock_ev_client* client;

    // since ev_io is the first member,
    // watcher `w` has the address of the 
    // start of the sock_ev_serv struct
    struct sock_ev_serv* server = (struct sock_ev_serv*) w;

    while (1) {
        client_fd = accept(server->fd, NULL, NULL);
        if( client_fd == -1 ) {
            if( errno != EAGAIN && errno != EWOULDBLOCK ) {
                g_warning("accept() failed errno=%i (%s)",  errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
            break;
        }
        printf("accepted a client\n");
        client = client_new(client_fd);
        client->server = server;
        client->index = array_push(&server->clients, client);
        ev_io_start(EV_A_ &client->io);
    }
}

int setnonblock(int fd) {
    int flags;

    flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

int unix_socket_init(struct sockaddr_un* socket_un, char* sock_path, int max_queue) {
    int fd;

    unlink(sock_path);

    // Setup a unix socket listener.
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == fd) {
        perror("echo server socket");
        exit(EXIT_FAILURE);
    }

    // Set it non-blocking
    if (-1 == setnonblock(fd)) {
        perror("echo server socket nonblock");
        exit(EXIT_FAILURE);
    }

    // Set it as unix socket
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
      perror("echo server bind");
      exit(EXIT_FAILURE);
    }

    if (-1 == listen(server->fd, max_queue)) {
      perror("listen");
      exit(EXIT_FAILURE);
    }
    return 0;
}

int main(void) {
    int max_queue = 128;
    struct sock_ev_serv server;
    struct ev_periodic every_few_seconds;
    // Create our single-loop for this single-thread application
    EV_P  = ev_default_loop(0);

    // Create unix socket in non-blocking fashion
    server_init(&server, "/tmp/libev-echo.sock", max_queue);

    // To be sure that we aren't actually blocking
    ev_periodic_init(&every_few_seconds, not_blocked, 0, 5, 0);
    ev_periodic_start(EV_A_ &every_few_seconds);

    // Get notified whenever the socket is ready to read
    ev_io_init(&server.io, server_cb, server.fd, EV_READ);
    ev_io_start(EV_A_ &server.io);

    // Run our loop, ostensibly forever
    printf("unix-socket-echo starting...\n");
    ev_loop(EV_A_ 0);

    // This point is only ever reached if the loop is manually exited
    close(server.fd);
    return EXIT_SUCCESS;
}

static void not_blocked(EV_P_ ev_periodic *w, int revents) {
    //printf("I'm not blocked\n");
}
