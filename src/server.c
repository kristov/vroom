#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <GL/glut.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ev.h>
#include <unistd.h>
#include "array-heap.h"
#include "vroom.pb-c.h"
#include "vrms_server.h"
#include "vrms_hmd.h"
#include "esm.h"
#include "opengl_stereo.h"

#define MAX_MSG_SIZE 1024

opengl_stereo* ostereo;
vrms_server_t* vrms_server;
vrms_hmd_t* vrms_hmd;

struct sock_ev_serv {
    ev_io io;
    int fd;
    struct sockaddr_un socket;
    int socket_len;
    array clients;
    vrms_server_t* vrms_server;
};

struct sock_ev_client {
    ev_io io;
    int fd;
    int index;
    struct sock_ev_serv* server;
};

uint32_t receive_create_scene(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateScene* cs_msg;

    if (NULL == vrms_server) {
        fprintf(stderr, "error server not initialized\n");
        return 0;
    }

    cs_msg = create_scene__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        fprintf(stderr, "error unpacking incoming message\n");
        *error = VRMS_INVALIDREQUEST;
        return 0;
    }

    id = vrms_create_scene(vrms_server, cs_msg->name);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_data_object(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error, int shm_fd) {
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

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, cs_msg->scene_id);

    id = vrms_create_data_object(vrms_scene, vrms_type, shm_fd, cs_msg->dtype, cs_msg->offset, cs_msg->size, cs_msg->stride);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_geometry_object(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateGeometryObject* cs_msg;
    cs_msg = create_geometry_object__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        printf("error unpacking incoming message\n");
        *error = VRMS_INVALIDREQUEST;
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, cs_msg->scene_id);

    id = vrms_create_geometry_object(vrms_scene, cs_msg->vertex_id, cs_msg->normal_id, cs_msg->index_id);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_mesh_color(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateMeshColor* cs_msg;
    cs_msg = create_mesh_color__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        printf("error unpacking incoming message\n");
        *error = VRMS_INVALIDREQUEST;
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, cs_msg->scene_id);

    id = vrms_create_mesh_color(vrms_scene, cs_msg->geometry_id, cs_msg->r, cs_msg->g, cs_msg->b, cs_msg->a);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_mesh_texture(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateMeshTexture* cs_msg;
    cs_msg = create_mesh_texture__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        printf("error unpacking incoming message\n");
        *error = VRMS_INVALIDREQUEST;
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, cs_msg->scene_id);

    id = vrms_create_mesh_texture(vrms_scene, cs_msg->geometry_id, cs_msg->uv_id, cs_msg->texture_id);
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

    re_msg.id = id;
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
        fprintf(stderr, "maximum message length exceeded: %d\n", length_r);
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
            id = receive_create_scene(client->server->vrms_server, in_buf, length_r, &error);
        break;
        case VRMS_DESTROYSCENE:
        break;
        case VRMS_CREATEDATAOBJECT:
            id = receive_create_data_object(client->server->vrms_server, in_buf, length_r, &error, shm_fd);
        break;
        case VRMS_DESTROYDATAOBJECT:
        break;
        case VRMS_CREATEGEOMETRYOBJECT:
            id = receive_create_geometry_object(client->server->vrms_server, in_buf, length_r, &error);
        break;
        case VRMS_CREATEMESHCOLOR:
            id = receive_create_mesh_color(client->server->vrms_server, in_buf, length_r, &error);
        break;
        case VRMS_CREATEMESHTEXTURE:
            id = receive_create_mesh_texture(client->server->vrms_server, in_buf, length_r, &error);
        break;
        default:
            id = 0;
            error = VRMS_INVALIDREQUEST;
        break;
    }

    send_reply(client->fd, id, error);
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
    //client->server = server;
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
        fprintf(stderr, "error creating server socket");
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

void* start_socket_thread(void* ptr) {
    int max_queue = 128;
    struct sock_ev_serv server;
    EV_P = ev_default_loop(0);

    server_init(&server, "/tmp/libev-echo.sock", max_queue);

    vrms_server = vrms_server_create();
    if (NULL == vrms_server) {
        fprintf(stderr, "error creating server\n");
        return NULL;
    }
    server.vrms_server = vrms_server;

    ev_io_init(&server.io, server_cb, server.fd, EV_READ);
    ev_io_start(EV_A_ &server.io);

    ev_loop(EV_A_ 0);

    close(server.fd);
    return NULL;
}

void* start_hmd_thread(void* ptr) {
    vrms_hmd = vrms_hmd_create();
    vrms_hmd_init(vrms_hmd);
    vrms_hmd_run(vrms_hmd);
    return NULL;
}

void draw_scene(opengl_stereo* ostereo) {
    vrms_server_draw_scene(vrms_server, ostereo->default_scene_shader_program_id, ostereo->projection_matrix, ostereo->view_matrix, ostereo->model_matrix);
}

GLvoid reshape(int w, int h) {
    opengl_stereo_reshape(ostereo, w, h);
}

GLvoid display(GLvoid) {
    opengl_stereo_display(ostereo);
    glutSwapBuffers();
}

void do_timer(int timer_event) {
    vrms_server_process_queues(vrms_server);
    if (vrms_hmd != NULL) {
        if (!pthread_mutex_trylock(vrms_hmd->matrix_lock)) {
            esmCopy(ostereo->hmd_matrix, vrms_hmd->matrix);
            pthread_mutex_unlock(vrms_hmd->matrix_lock);
        }
    }
    glutPostRedisplay();
    glutTimerFunc(10, do_timer, 1);
}

void initWindowingSystem(int *argc, char **argv, int width, int height) {
    glutInit(argc, argv);
    glutInitWindowSize(width, height);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
    glutCreateWindow("Stereo Test");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(10, do_timer, 1);
}

void init(int *argc, char **argv) {
    int width = 1152;
    int height = 648;
    double physical_width = 1.347;

    initWindowingSystem(argc, argv, width, height);
    ostereo = opengl_stereo_create(width, height, physical_width);
    ostereo->draw_scene_function = &draw_scene;
}



int32_t main(int argc, char **argv) {
    pthread_t socket_thread;
    pthread_t hmd_thread;
    int32_t thread_ret;

    thread_ret = pthread_create(&socket_thread, NULL, start_socket_thread, NULL);
    if (thread_ret != 0) {
        fprintf(stderr, "unable to start socket thread\n");
        exit(1);
    }

    thread_ret = pthread_create(&hmd_thread, NULL, start_hmd_thread, NULL);
    if (thread_ret != 0) {
        fprintf(stderr, "unable to start hmd thread\n");
        exit(1);
    }

    init(&argc, argv);
    glutMainLoop();

    pthread_join(socket_thread, NULL);
    pthread_join(hmd_thread, NULL);
    return 0;
}
