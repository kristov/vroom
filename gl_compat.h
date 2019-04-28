#ifndef GL_COMPAT_H
#define GL_COMPAT_H

#ifdef RASPBERRYPI
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif EGLGBM
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define GL_STACK_UNDERFLOW 0x0504
#define GL_STACK_OVERFLOW  0x0503
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_RGB8            0x8051
#define GL_RGBA8           0x8058
#define GL_BGRA            0x80E1
#define GL_BGRA8           0x93A1
#define GL_TEXTURE_WRAP_R  0x8072
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL  0x813D
#else /* GLUT */
#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#endif /* RASPBERRYPI EGLGBM */

#endif
