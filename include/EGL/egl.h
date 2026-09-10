#ifndef __egl_h_
#define __egl_h_

#include <EGL/eglplatform.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int EGLBoolean;
typedef unsigned int EGLenum;
typedef void *EGLConfig;
typedef void *EGLContext;
typedef void *EGLDisplay;
typedef void *EGLSurface;
typedef void *EGLClientBuffer;

#define EGL_FALSE                       0
#define EGL_TRUE                        1

#define EGL_DEFAULT_DISPLAY             ((EGLNativeDisplayType)0)
#define EGL_NO_CONTEXT                  ((EGLContext)0)
#define EGL_NO_DISPLAY                  ((EGLDisplay)0)
#define EGL_NO_SURFACE                  ((EGLSurface)0)
#define EGL_DONT_CARE                   ((EGLint)-1)

#define EGL_SUCCESS                     0x3000
#define EGL_NOT_INITIALIZED             0x3001
#define EGL_BAD_ACCESS                  0x3002
#define EGL_BAD_ALLOC                   0x3003
#define EGL_BAD_ATTRIBUTE               0x3004
#define EGL_BAD_CONFIG                  0x3005
#define EGL_BAD_CONTEXT                 0x3006
#define EGL_BAD_CURRENT_SURFACE         0x3007
#define EGL_BAD_DISPLAY                 0x3008
#define EGL_BAD_MATCH                   0x3009
#define EGL_BAD_NATIVE_PIXMAP           0x300A
#define EGL_BAD_NATIVE_WINDOW           0x300B
#define EGL_BAD_PARAMETER               0x300C
#define EGL_BAD_SURFACE                 0x300D

#define EGL_BUFFER_SIZE                 0x3020
#define EGL_RED_SIZE                    0x3024
#define EGL_GREEN_SIZE                  0x3023
#define EGL_BLUE_SIZE                   0x3022
#define EGL_ALPHA_SIZE                  0x3021
#define EGL_DEPTH_SIZE                  0x3025
#define EGL_STENCIL_SIZE                0x3026
#define EGL_CONFIG_CAVEAT               0x3027
#define EGL_CONFIG_ID                   0x3028
#define EGL_LEVEL                       0x3029
#define EGL_MAX_PBUFFER_HEIGHT          0x302A
#define EGL_MAX_PBUFFER_PIXELS          0x302B
#define EGL_MAX_PBUFFER_WIDTH           0x302C
#define EGL_NATIVE_RENDERABLE           0x302D
#define EGL_NATIVE_VISUAL_ID            0x302E
#define EGL_NATIVE_VISUAL_TYPE          0x302F
#define EGL_SAMPLES                     0x3031
#define EGL_SAMPLE_BUFFERS              0x3032
#define EGL_SURFACE_TYPE                0x3033
#define EGL_NONE                        0x3038
#define EGL_RENDERABLE_TYPE             0x3040
#define EGL_WINDOW_BIT                  0x0004
#define EGL_OPENGL_ES_BIT               0x0001
#define EGL_OPENGL_ES2_BIT              0x0004
#define EGL_OPENGL_BIT                  0x0008

#define EGL_OPENGL_ES_API               0x30A0
#define EGL_OPENVG_API                  0x30A1
#define EGL_OPENGL_API                  0x30A2

#define EGL_CONTEXT_CLIENT_VERSION      0x3098
#define EGL_PLATFORM_WAYLAND_KHR        0x31D8
#define EGL_PLATFORM_WAYLAND_EXT        0x31D8

EGLAPI EGLint EGLAPIENTRY eglGetError(void);
EGLAPI EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id);
EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy);
EGLAPI EGLBoolean EGLAPIENTRY eglBindAPI(EGLenum api);
EGLAPI EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
EGLAPI EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay dpy, EGLSurface surface);
EGLAPI EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
EGLAPI EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
EGLAPI EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval);
typedef void (*__eglMustCastToProperFunctionPointerType)(void);
EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY eglGetProcAddress(const char *procname);

#ifdef __cplusplus
}
#endif

#endif
