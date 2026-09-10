#ifndef __eglext_h_
#define __eglext_h_

#include <EGL/egl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef EGLDisplay (EGLAPIENTRYP PFNEGLGETPLATFORMDISPLAYEXTPROC) (EGLenum platform, void *native_display, const EGLint *attrib_list);
typedef EGLSurface (EGLAPIENTRYP PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC) (EGLDisplay dpy, EGLConfig config, void *native_window, const EGLint *attrib_list);

#ifdef __cplusplus
}
#endif

#endif
