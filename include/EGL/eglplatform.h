#ifndef __eglplatform_h_
#define __eglplatform_h_

#include <KHR/khrplatform.h>

#ifndef EGLAPI
#define EGLAPI KHRONOS_APICALL
#endif

#ifndef EGLAPIENTRY
#define EGLAPIENTRY KHRONOS_APIENTRY
#endif

#ifndef EGLAPIENTRYP
#define EGLAPIENTRYP EGLAPIENTRY *
#endif

struct wl_display;
struct wl_egl_window;

typedef struct wl_display     *EGLNativeDisplayType;
typedef struct wl_egl_window  *EGLNativeWindowType;
typedef void                  *EGLNativePixmapType;

typedef khronos_int32_t EGLint;

#endif
