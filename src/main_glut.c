#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <GL/glut.h>
#include "vrms_server_socket.h"

GLvoid reshape(int w, int h) {
    vrms_server_socket_reshape(w, h);
}

GLvoid display(GLvoid) {
    vrms_server_socket_display();
    glutSwapBuffers();
}

void do_timer(int timer_event) {
    vrms_server_socket_process();
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
    vrms_server_socket_init(width, height, physical_width);
}

int32_t main(int argc, char **argv) {

    init(&argc, argv);
    glutMainLoop();

    vrms_server_socket_end();
    return 0;
}
