#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>

#include <wayland-client.h>
#include <wayland-egl.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <mpv/client.h>
#include <mpv/render_gl.h>

#include "wlr-layer-shell-protocol.h"

// Satisfies linker reference in wlr-layer-shell-protocol.c without needing external xdg-shell library
const struct wl_interface xdg_popup_interface = {
    .name = "xdg_popup",
    .version = 1,
};

#define MAX_OUTPUTS 16

typedef struct {
    struct wl_output *output;
    uint32_t id;
    char name[64];
    char description[128];
    int width;
    int height;
    int refresh_rate;
    bool connected;
} OutputInfo;

typedef struct {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct zwlr_layer_shell_v1 *layer_shell;

    OutputInfo outputs[MAX_OUTPUTS];
    int output_count;
    OutputInfo *target_output;

    struct wl_surface *wl_surface;
    struct zwlr_layer_surface_v1 *layer_surface;
    struct wl_egl_window *egl_window;

    EGLDisplay egl_display;
    EGLConfig egl_config;
    EGLContext egl_context;
    EGLSurface egl_surface;

    mpv_handle *mpv;
    mpv_render_context *mpv_gl;

    int surface_width;
    int surface_height;
    bool configured;
    bool should_exit;
    bool wakeup_render;

    char target_output_name[64];
    char scaling_mode[32];
    char video_path[1024];
    bool start_paused;
} AppState;

static AppState g_app;

static void *get_proc_address_mpv(void *ctx, const char *name) {
    (void)ctx;
    return (void *)eglGetProcAddress(name);
}

static void on_mpv_render_update(void *ctx) {
    AppState *app = (AppState *)ctx;
    app->wakeup_render = true;
}

static void handle_layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *surface,
                                          uint32_t serial, uint32_t w, uint32_t h) {
    AppState *app = (AppState *)data;
    zwlr_layer_surface_v1_ack_configure(surface, serial);

    if (w == 0 || h == 0) {
        if (app->target_output && app->target_output->width > 0 && app->target_output->height > 0) {
            w = app->target_output->width;
            h = app->target_output->height;
        } else {
            w = 1920;
            h = 1080;
        }
    }

    if (!app->configured || app->surface_width != (int)w || app->surface_height != (int)h) {
        app->surface_width = w;
        app->surface_height = h;
        if (app->egl_window) {
            wl_egl_window_resize(app->egl_window, w, h, 0, 0);
        }
        app->configured = true;
        printf("[engine] Layer surface configured: %dx%d\n", w, h);
        fflush(stdout);
    }
}

static void handle_layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *surface) {
    (void)surface;
    AppState *app = (AppState *)data;
    printf("[engine] Layer surface closed by compositor\n");
    app->should_exit = true;
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = handle_layer_surface_configure,
    .closed = handle_layer_surface_closed,
};

static void handle_output_geometry(void *data, struct wl_output *output, int32_t x, int32_t y,
                                   int32_t phys_w, int32_t phys_h, int32_t subpixel,
                                   const char *make, const char *model, int32_t transform) {
    (void)output; (void)x; (void)y; (void)phys_w; (void)phys_h;
    (void)subpixel; (void)transform;
    OutputInfo *info = (OutputInfo *)data;
    snprintf(info->description, sizeof(info->description), "%s %s", make ? make : "", model ? model : "");
}

static void handle_output_mode(void *data, struct wl_output *output, uint32_t flags,
                               int32_t width, int32_t height, int32_t refresh) {
    (void)output;
    OutputInfo *info = (OutputInfo *)data;
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        info->width = width;
        info->height = height;
        info->refresh_rate = refresh;
    }
}

static void handle_output_done(void *data, struct wl_output *output) {
    (void)output;
    OutputInfo *info = (OutputInfo *)data;
    info->connected = true;
    printf("[engine] Output detected: %s (%s) %dx%d @ %.2f Hz\n",
           info->name[0] ? info->name : "unknown",
           info->description[0] ? info->description : "display",
           info->width, info->height, (double)info->refresh_rate / 1000.0);
    fflush(stdout);
}

static void handle_output_scale(void *data, struct wl_output *output, int32_t factor) {
    (void)data; (void)output; (void)factor;
}

static void handle_output_name(void *data, struct wl_output *output, const char *name) {
    (void)output;
    OutputInfo *info = (OutputInfo *)data;
    if (name) {
        strncpy(info->name, name, sizeof(info->name) - 1);
        info->name[sizeof(info->name) - 1] = '\0';
    }
}

static void handle_output_description(void *data, struct wl_output *output, const char *desc) {
    (void)output;
    OutputInfo *info = (OutputInfo *)data;
    if (desc && !info->description[0]) {
        strncpy(info->description, desc, sizeof(info->description) - 1);
        info->description[sizeof(info->description) - 1] = '\0';
    }
}

static const struct wl_output_listener output_listener = {
    .geometry = handle_output_geometry,
    .mode = handle_output_mode,
    .done = handle_output_done,
    .scale = handle_output_scale,
    .name = handle_output_name,
    .description = handle_output_description,
};

static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface, uint32_t version) {
    AppState *app = (AppState *)data;

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        app->compositor = (struct wl_compositor *)wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        uint32_t bind_version = version < 4 ? version : 4;
        app->layer_shell = (struct zwlr_layer_shell_v1 *)wl_registry_bind(
            registry, name, &zwlr_layer_shell_v1_interface, bind_version);
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
        if (app->output_count < MAX_OUTPUTS) {
            OutputInfo *info = &app->outputs[app->output_count++];
            info->id = name;
            uint32_t bind_version = version < 4 ? version : 4;
            info->output = (struct wl_output *)wl_registry_bind(registry, name, &wl_output_interface, bind_version);
            wl_output_add_listener(info->output, &output_listener, info);
        }
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
    (void)data; (void)registry; (void)name;
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static void apply_video_scale(AppState *app) {
    int target_w = app->surface_width > 0 ? app->surface_width : 1920;
    int target_h = app->surface_height > 0 ? app->surface_height : 1080;

    int64_t vid_w = 0, vid_h = 0;
    mpv_get_property(app->mpv, "video-params/w", MPV_FORMAT_INT64, &vid_w);
    mpv_get_property(app->mpv, "video-params/h", MPV_FORMAT_INT64, &vid_h);

    printf("[engine] Configuring scale mode '%s' for %ldx%ld on target surface %dx%d\n",
           app->scaling_mode, (long)vid_w, (long)vid_h, target_w, target_h);

    // Reset software video filter (prevents ffmpeg hardware surface conversion errors)
    mpv_set_property_string(app->mpv, "vf", "");

    // Reset zoom and centering
    mpv_set_property_string(app->mpv, "video-zoom", "0");
    mpv_set_property_string(app->mpv, "video-align-x", "0.0");
    mpv_set_property_string(app->mpv, "video-align-y", "0.0");
    mpv_set_property_string(app->mpv, "video-unscaled", "no");

    if (strcmp(app->scaling_mode, "fill") == 0) {
        // Fill: expands video to cover the full screen without black bars (panscan 1.0)
        mpv_set_property_string(app->mpv, "panscan", "1.0");
        mpv_set_property_string(app->mpv, "keepaspect", "yes");
    } else if (strcmp(app->scaling_mode, "stretch") == 0) {
        // Stretch: stretches the video to cover all 4 edges (ignores aspect ratio)
        mpv_set_property_string(app->mpv, "panscan", "0.0");
        mpv_set_property_string(app->mpv, "keepaspect", "no");
    } else {
        // Fit (default): preserves exact aspect ratio, no unwanted zoom, full video visible
        mpv_set_property_string(app->mpv, "panscan", "0.0");
        mpv_set_property_string(app->mpv, "keepaspect", "yes");
    }
    fflush(stdout);
}

static void sigint_handler(int sig) {
    (void)sig;
    g_app.should_exit = true;
}

static void print_usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s [options] <video-path>\n\n"
            "Options:\n"
            "  --output <name>        Target output port (e.g. DVI-D-1, DP-1, or '*'). Default: auto\n"
            "  --scaling <mode>       Scaling mode: fit (default), fill, stretch\n"
            "  --pause                Start playback paused\n"
            "  --help                 Show this help message\n\n",
            prog);
}

int main(int argc, char **argv) {
    memset(&g_app, 0, sizeof(g_app));
    strcpy(g_app.scaling_mode, "fit");

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            strncpy(g_app.target_output_name, argv[++i], sizeof(g_app.target_output_name) - 1);
        } else if (strcmp(argv[i], "--scaling") == 0 && i + 1 < argc) {
            strncpy(g_app.scaling_mode, argv[++i], sizeof(g_app.scaling_mode) - 1);
        } else if (strcmp(argv[i], "--pause") == 0) {
            g_app.start_paused = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (argv[i][0] != '-') {
            strncpy(g_app.video_path, argv[i], sizeof(g_app.video_path) - 1);
        }
    }

    if (g_app.video_path[0] == '\0') {
        fprintf(stderr, "[engine] Error: No video file specified.\n");
        print_usage(argv[0]);
        return 1;
    }

    if (access(g_app.video_path, R_OK) != 0) {
        fprintf(stderr, "[engine] Error: Cannot read video file '%s': %s\n",
                g_app.video_path, strerror(errno));
        return 1;
    }

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    printf("[engine] Connecting to Wayland compositor...\n");
    g_app.display = wl_display_connect(NULL);
    if (!g_app.display) {
        fprintf(stderr, "[engine] Error: Failed to connect to Wayland display. Is WAYLAND_DISPLAY set?\n");
        return 1;
    }

    g_app.registry = wl_display_get_registry(g_app.display);
    wl_registry_add_listener(g_app.registry, &registry_listener, &g_app);

    // Initial roundtrips to populate globals & outputs
    wl_display_roundtrip(g_app.display);
    wl_display_roundtrip(g_app.display);

    if (!g_app.compositor) {
        fprintf(stderr, "[engine] Error: Compositor does not support wl_compositor.\n");
        return 1;
    }
    if (!g_app.layer_shell) {
        fprintf(stderr, "[engine] Error: Compositor does not support zwlr_layer_shell_v1.\n");
        return 1;
    }

    // Select target output
    if (g_app.target_output_name[0] != '\0' && strcmp(g_app.target_output_name, "*") != 0) {
        for (int i = 0; i < g_app.output_count; i++) {
            if (strcmp(g_app.outputs[i].name, g_app.target_output_name) == 0) {
                g_app.target_output = &g_app.outputs[i];
                break;
            }
        }
        if (!g_app.target_output) {
            fprintf(stderr, "[engine] Warning: Requested output '%s' not found. Falling back to primary output.\n",
                    g_app.target_output_name);
        }
    }

    if (!g_app.target_output) {
        for (int i = 0; i < g_app.output_count; i++) {
            if (g_app.outputs[i].connected && g_app.outputs[i].width > 0) {
                g_app.target_output = &g_app.outputs[i];
                break;
            }
        }
    }

    if (g_app.target_output) {
        printf("[engine] Targeted output: %s (%dx%d)\n",
               g_app.target_output->name[0] ? g_app.target_output->name : "primary",
               g_app.target_output->width, g_app.target_output->height);
    } else {
        printf("[engine] No specific output selected. Attaching layer-shell to default display.\n");
    }

    // Create Wayland surface & wlr-layer-shell surface
    g_app.wl_surface = wl_compositor_create_surface(g_app.compositor);
    if (!g_app.wl_surface) {
        fprintf(stderr, "[engine] Error: Failed to create wl_surface.\n");
        return 1;
    }

    struct wl_output *target_wl_output = g_app.target_output ? g_app.target_output->output : NULL;
    g_app.layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        g_app.layer_shell,
        g_app.wl_surface,
        target_wl_output,
        ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND,
        "cosmic-wallpaper"
    );

    if (!g_app.layer_surface) {
        fprintf(stderr, "[engine] Error: Failed to create zwlr_layer_surface_v1.\n");
        return 1;
    }

    zwlr_layer_surface_v1_add_listener(g_app.layer_surface, &layer_surface_listener, &g_app);

    // Anchor to all four edges for full wallpaper coverage
    uint32_t anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                      ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                      ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                      ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
    zwlr_layer_surface_v1_set_anchor(g_app.layer_surface, anchor);
    zwlr_layer_surface_v1_set_size(g_app.layer_surface, 0, 0);
    zwlr_layer_surface_v1_set_exclusive_zone(g_app.layer_surface, -1);
    zwlr_layer_surface_v1_set_keyboard_interactivity(g_app.layer_surface,
                                                     ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);

    // Initial commit to request configure event
    wl_surface_commit(g_app.wl_surface);
    wl_display_roundtrip(g_app.display);

    if (!g_app.configured) {
        wl_display_roundtrip(g_app.display);
    }

    int init_w = g_app.surface_width > 0 ? g_app.surface_width : 1920;
    int init_h = g_app.surface_height > 0 ? g_app.surface_height : 1080;
    g_app.surface_width = init_w;
    g_app.surface_height = init_h;

    // Create EGL window
    g_app.egl_window = wl_egl_window_create(g_app.wl_surface, init_w, init_h);
    if (!g_app.egl_window) {
        fprintf(stderr, "[engine] Error: Failed to create wl_egl_window.\n");
        return 1;
    }

    // Initialize EGL
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

    if (eglGetPlatformDisplayEXT) {
        g_app.egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_EXT, g_app.display, NULL);
    }
    if (!g_app.egl_display) {
        g_app.egl_display = eglGetDisplay((EGLNativeDisplayType)g_app.display);
    }

    if (g_app.egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "[engine] Error: Failed to get EGL display.\n");
        return 1;
    }

    EGLint major = 0, minor = 0;
    if (!eglInitialize(g_app.egl_display, &major, &minor)) {
        fprintf(stderr, "[engine] Error: eglInitialize failed (0x%x).\n", eglGetError());
        return 1;
    }
    printf("[engine] EGL initialized (version %d.%d)\n", major, minor);

    eglBindAPI(EGL_OPENGL_API);

    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLint num_configs = 0;
    if (!eglChooseConfig(g_app.egl_display, config_attribs, &g_app.egl_config, 1, &num_configs) || num_configs < 1) {
        // Fallback to OpenGL ES 2
        eglBindAPI(EGL_OPENGL_ES_API);
        config_attribs[11] = EGL_OPENGL_ES2_BIT;
        if (!eglChooseConfig(g_app.egl_display, config_attribs, &g_app.egl_config, 1, &num_configs) || num_configs < 1) {
            fprintf(stderr, "[engine] Error: eglChooseConfig failed to find a valid config.\n");
            return 1;
        }
    }

    EGLint ctx_attribs[] = { EGL_NONE };
    g_app.egl_context = eglCreateContext(g_app.egl_display, g_app.egl_config, EGL_NO_CONTEXT, ctx_attribs);
    if (g_app.egl_context == EGL_NO_CONTEXT) {
        fprintf(stderr, "[engine] Error: Failed to create EGL context (0x%x).\n", eglGetError());
        return 1;
    }

    g_app.egl_surface = eglCreateWindowSurface(g_app.egl_display, g_app.egl_config,
                                               (EGLNativeWindowType)g_app.egl_window, NULL);
    if (g_app.egl_surface == EGL_NO_SURFACE) {
        fprintf(stderr, "[engine] Error: Failed to create EGL window surface (0x%x).\n", eglGetError());
        return 1;
    }

    if (!eglMakeCurrent(g_app.egl_display, g_app.egl_surface, g_app.egl_surface, g_app.egl_context)) {
        fprintf(stderr, "[engine] Error: eglMakeCurrent failed (0x%x).\n", eglGetError());
        return 1;
    }

    eglSwapInterval(g_app.egl_display, 1);

    // Initialize libmpv
    g_app.mpv = mpv_create();
    if (!g_app.mpv) {
        fprintf(stderr, "[engine] Error: Failed to create mpv handle.\n");
        return 1;
    }

    mpv_set_option_string(g_app.mpv, "terminal", "yes");
    mpv_set_option_string(g_app.mpv, "msg-level", "all=warn");
    mpv_set_option_string(g_app.mpv, "loop-file", "inf");
    mpv_set_option_string(g_app.mpv, "audio", "no");
    mpv_set_option_string(g_app.mpv, "hwdec", "auto-safe");
    mpv_set_option_string(g_app.mpv, "vo", "libmpv");
    mpv_set_option_string(g_app.mpv, "gpu-context", "wayland");
    mpv_set_option_string(g_app.mpv, "opengl-pbo", "no");
    mpv_set_option_string(g_app.mpv, "video-sync", "display-resample");
    mpv_set_option_string(g_app.mpv, "image-display-duration", "inf");

    if (g_app.start_paused) {
        mpv_set_option_string(g_app.mpv, "pause", "yes");
    }

    if (mpv_initialize(g_app.mpv) < 0) {
        fprintf(stderr, "[engine] Error: Failed to initialize mpv.\n");
        return 1;
    }

    mpv_opengl_init_params gl_init_params = {
        .get_proc_address = get_proc_address_mpv,
        .get_proc_address_ctx = NULL,
    };

    mpv_render_param render_params[] = {
        { MPV_RENDER_PARAM_API_TYPE, MPV_RENDER_API_TYPE_OPENGL },
        { MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params },
        { MPV_RENDER_PARAM_WL_DISPLAY, g_app.display },
        { MPV_RENDER_PARAM_INVALID, NULL }
    };

    if (mpv_render_context_create(&g_app.mpv_gl, g_app.mpv, render_params) < 0) {
        fprintf(stderr, "[engine] Error: Failed to create mpv render context.\n");
        return 1;
    }

    mpv_render_context_set_update_callback(g_app.mpv_gl, on_mpv_render_update, &g_app);
    mpv_observe_property(g_app.mpv, 1, "video-params/w", MPV_FORMAT_INT64);
    mpv_observe_property(g_app.mpv, 2, "video-params/h", MPV_FORMAT_INT64);

    // Load video
    const char *load_cmd[] = { "loadfile", g_app.video_path, NULL };
    if (mpv_command(g_app.mpv, load_cmd) < 0) {
        fprintf(stderr, "[engine] Error: Failed to load file '%s'.\n", g_app.video_path);
        return 1;
    }

    printf("[engine] Video loading: %s\n", g_app.video_path);
    fflush(stdout);

    // Configure stdin to non-blocking for IPC commands
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    int wl_fd = wl_display_get_fd(g_app.display);
    struct pollfd fds[2] = {
        { .fd = wl_fd, .events = POLLIN },
        { .fd = STDIN_FILENO, .events = POLLIN }
    };

    bool video_scaled = false;

    while (!g_app.should_exit) {
        // Prepare Wayland events
        while (wl_display_prepare_read(g_app.display) != 0) {
            wl_display_dispatch_pending(g_app.display);
        }
        wl_display_flush(g_app.display);

        int timeout = g_app.wakeup_render ? 0 : 16;
        int ret = poll(fds, 2, timeout);

        if (ret > 0) {
            if (fds[0].revents & POLLIN) {
                wl_display_read_events(g_app.display);
            } else {
                wl_display_cancel_read(g_app.display);
            }

            if (fds[1].revents & POLLIN) {
                char cmd_buf[256];
                ssize_t n = read(STDIN_FILENO, cmd_buf, sizeof(cmd_buf) - 1);
                if (n > 0) {
                    cmd_buf[n] = '\0';
                    char *line = strtok(cmd_buf, "\r\n");
                    while (line) {
                        if (strcmp(line, "PAUSE") == 0) {
                            printf("[engine] Command: PAUSE\n");
                            mpv_set_property_string(g_app.mpv, "pause", "yes");
                        } else if (strcmp(line, "RESUME") == 0) {
                            printf("[engine] Command: RESUME\n");
                            mpv_set_property_string(g_app.mpv, "pause", "no");
                        } else if (strcmp(line, "TOGGLE") == 0) {
                            printf("[engine] Command: TOGGLE\n");
                            int paused = 0;
                            mpv_get_property(g_app.mpv, "pause", MPV_FORMAT_FLAG, &paused);
                            int new_state = !paused;
                            mpv_set_property(g_app.mpv, "pause", MPV_FORMAT_FLAG, &new_state);
                        } else if (strcmp(line, "STOP") == 0 || strcmp(line, "QUIT") == 0) {
                            printf("[engine] Command: QUIT\n");
                            g_app.should_exit = true;
                            break;
                        } else if (strncmp(line, "LOAD ", 5) == 0) {
                            const char *new_path = line + 5;
                            printf("[engine] Command: LOAD %s\n", new_path);
                            const char *cmd[] = { "loadfile", new_path, NULL };
                            mpv_command(g_app.mpv, cmd);
                            video_scaled = false;
                        } else if (strncmp(line, "SCALING ", 8) == 0) {
                            const char *new_mode = line + 8;
                            printf("[engine] Command: SCALING %s\n", new_mode);
                            strncpy(g_app.scaling_mode, new_mode, sizeof(g_app.scaling_mode) - 1);
                            apply_video_scale(&g_app);
                        }
                        line = strtok(NULL, "\r\n");
                    }
                    fflush(stdout);
                }
            }
        } else {
            wl_display_cancel_read(g_app.display);
        }

        wl_display_dispatch_pending(g_app.display);

        // Process mpv events
        while (1) {
            mpv_event *event = mpv_wait_event(g_app.mpv, 0);
            if (event->event_id == MPV_EVENT_NONE) break;
            if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
                if (!video_scaled) {
                    apply_video_scale(&g_app);
                    video_scaled = true;
                }
            } else if (event->event_id == MPV_EVENT_FILE_LOADED) {
                apply_video_scale(&g_app);
                video_scaled = true;
            } else if (event->event_id == MPV_EVENT_SHUTDOWN) {
                g_app.should_exit = true;
            }
        }

        // Render frame if ready
        if (g_app.wakeup_render && g_app.configured) {
            uint64_t flags = mpv_render_context_update(g_app.mpv_gl);
            if (flags & MPV_RENDER_UPDATE_FRAME) {
                mpv_opengl_fbo fbo = {
                    .fbo = 0,
                    .w = g_app.surface_width,
                    .h = g_app.surface_height,
                    .internal_format = 0,
                };
                int flip_y = 1;

                mpv_render_param render_draw_params[] = {
                    { MPV_RENDER_PARAM_OPENGL_FBO, &fbo },
                    { MPV_RENDER_PARAM_FLIP_Y, &flip_y },
                    { MPV_RENDER_PARAM_INVALID, NULL }
                };

                mpv_render_context_render(g_app.mpv_gl, render_draw_params);
                eglSwapBuffers(g_app.egl_display, g_app.egl_surface);
                mpv_render_context_report_swap(g_app.mpv_gl);
            }
            g_app.wakeup_render = false;
        }
    }

    printf("[engine] Shutting down cleanly...\n");
    if (g_app.mpv_gl) mpv_render_context_free(g_app.mpv_gl);
    if (g_app.mpv) mpv_terminate_destroy(g_app.mpv);

    if (g_app.egl_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_app.egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (g_app.egl_surface != EGL_NO_SURFACE) eglDestroySurface(g_app.egl_display, g_app.egl_surface);
        if (g_app.egl_context != EGL_NO_CONTEXT) eglDestroyContext(g_app.egl_display, g_app.egl_context);
        eglTerminate(g_app.egl_display);
    }

    if (g_app.egl_window) wl_egl_window_destroy(g_app.egl_window);
    if (g_app.layer_surface) zwlr_layer_surface_v1_destroy(g_app.layer_surface);
    if (g_app.wl_surface) wl_surface_destroy(g_app.wl_surface);
    if (g_app.layer_shell) zwlr_layer_shell_v1_destroy(g_app.layer_shell);
    if (g_app.compositor) wl_compositor_destroy(g_app.compositor);

    for (int i = 0; i < g_app.output_count; i++) {
        if (g_app.outputs[i].output) wl_output_destroy(g_app.outputs[i].output);
    }

    if (g_app.registry) wl_registry_destroy(g_app.registry);
    if (g_app.display) wl_display_disconnect(g_app.display);

    printf("[engine] Engine terminated.\n");
    return 0;
}
