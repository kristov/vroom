#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <GL/glut.h>
#include "vrms_runtime.h"

vrms_runtime_t* vrms_runtime;

GLvoid reshape(int w, int h) {
    vrms_runtime_reshape(vrms_runtime, w, h);
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
