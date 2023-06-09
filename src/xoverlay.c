/*
 * overlay.c
 *
 *  Created on: Nov 9, 2017
 *      Author: nullifiedcat
 */

#include "internal/drawglx.h"
#include <GL/glx.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xfixes.h>
#include <string.h>
#include <stdio.h>
#include <xoverlay.h>
#include <dlfcn.h> // dlsym

int event_ShapeNotify;
int event_ShapeError;
struct xoverlay_library xoverlay_library;
typedef void (*glXSwapBuffers_t)(Display *dpy, GLXDrawable drawable);
glXSwapBuffers_t glXSwapBuffersfn;

int xoverlay_init()
{
    memset(&xoverlay_library, 0, sizeof(struct xoverlay_library));

    xoverlay_library.display = XOpenDisplay(NULL);
    if (xoverlay_library.display == NULL)
        return -2;

    xoverlay_library.screen = DefaultScreen(xoverlay_library.display);
    xoverlay_library.width = DisplayWidth(xoverlay_library.display, xoverlay_library.screen);
    xoverlay_library.height = DisplayHeight(xoverlay_library.display, xoverlay_library.screen);

    if (!XShapeQueryExtension(xoverlay_library.display, &event_ShapeNotify, &event_ShapeError))
        return -3;

    if (xoverlay_glx_init() < 0)
        return -4;

    if (xoverlay_glx_create_window() < 0)
        return -5;

    glXSwapBuffersfn = (glXSwapBuffers_t)dlsym((void *)0xFFFFFFFF, "glXSwapBuffers");

    if (!glXSwapBuffersfn)
        return -6;

    xoverlay_library.init = 1;

    return 0;
int xoverlay_initialized = 0;
int xoverlay_visibility = 0;
int xoverlay_drawing = 0;

void xoverlay_destroy()
{
    if (!xoverlay_initialized)
        return;

    XDestroyWindow(xoverlay_library.display, xoverlay_library.window);
    XCloseDisplay(xoverlay_library.display);
    xoverlay_glx_destroy();
    xoverlay_initialized = 0;
}

void xoverlay_show()
{
    if (xoverlay_visibility)
        return;

    XMapWindow(xoverlay_library.display, xoverlay_library.window);
    xoverlay_visibility = 1;
}

void xoverlay_hide()
{
    if (!xoverlay_visibility)
        return;

    XUnmapWindow(xoverlay_library.display, xoverlay_library.window);
    xoverlay_visibility = 0;
}

void xoverlay_draw_begin()
{
    if (xoverlay_drawing)
        return;

    xoverlay_drawing = 1;
    glXMakeCurrent(xoverlay_library.display, xoverlay_library.window, glx_state.context);
    // Optional: glClear(GL_COLOR_BUFFER_BIT);
}

void xoverlay_draw_end()
{
    if (!xoverlay_drawing)
        return;

    glXSwapBuffersfn(xoverlay_library.display, xoverlay_library.window);
    xoverlay_drawing = 0;
}
