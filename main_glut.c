#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <GL/glut.h>
#include "gl-matrix.h"
#include "runtime.h"

#define PI    3.141593f
#define TWOPI 6.283185f

vrms_runtime_t* vrms_runtime;
float view_matrix[16];

GLvoid reshape(int w, int h) {
    vrms_runtime_reshape(vrms_runtime, w, h);
}

void motion(int x, int y) {
    float xpct;
    float ypct;
    float xangle;
    float yangle;

    xpct = (float)x / (float)vrms_runtime->w;
    ypct = (float)y / (float)vrms_runtime->h;

    xangle = TWOPI * xpct;
    yangle = (PI * ypct) - (PI / 2);

    mat4_identity(view_matrix);
    mat4_rotateX(view_matrix, yangle);
    mat4_rotateY(view_matrix, xangle);
    vrms_runtime_update_system_matrix(vrms_runtime, VRMS_MATRIX_HEAD, VRMS_UPDATE_SET, view_matrix);
}

GLvoid display(GLvoid) {
    vrms_runtime_display(vrms_runtime);
    glutSwapBuffers();
}

void do_timer(int timer_event) {
    vrms_runtime_process(vrms_runtime);
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
    glutPassiveMotionFunc(motion);
    glutTimerFunc(10, do_timer, 1);

    //const GLubyte* extensions = glGetString(GL_EXTENSIONS);
    //fprintf(stderr, "extensions: %s\n", extensions);

    const GLubyte* shaderv = glGetString(GL_SHADING_LANGUAGE_VERSION);
    fprintf(stderr, "shaderv: %s\n", shaderv);
}

void init(int *argc, char **argv) {
    int width = 1152;
    int height = 648;
    double physical_width = 1.347;

    initWindowingSystem(argc, argv, width, height);
    vrms_runtime = vrms_runtime_init(width, height, physical_width);
}

int32_t main(int argc, char **argv) {
    init(&argc, argv);
    glutMainLoop();

    vrms_runtime_end(vrms_runtime);
    return 0;
}
