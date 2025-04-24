#include "platform.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <X11/Xutil.h>
#include <stddef.h>
#include <stdio.h>

static Display *display;
static Window window;
static GLXContext glc;
static Atom delete_window_atom;

void
create_window(int width, int height, const char *title)
{
	display = XOpenDisplay(NULL);
	Window root = DefaultRootWindow(display);
	GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	XVisualInfo *vi = glXChooseVisual(display, 0, att);
	Colormap cmap = XCreateColormap(display, root, vi->visual, AllocNone);
	XSetWindowAttributes swa = {
		.colormap = cmap,
		.event_mask = ExposureMask | KeyPressMask,
	};
	window = XCreateWindow(display, root, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	XMapWindow(display, window);
	XStoreName(display, window, title);
	glc = glXCreateContext(display, vi, NULL, GL_TRUE);
	glXMakeCurrent(display, window, glc);
	delete_window_atom = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &delete_window_atom, 1);
}

static SpecialKey
keycode_to_specialkey(int keycode)
{
	switch (keycode) {
		case XK_Left: return KEY_ARROW_LEFT;
		case XK_Right: return KEY_ARROW_RIGHT;
		case XK_BackSpace: return KEY_BACKSPACE;
		case XK_Control_L: return KEY_CONTROL;
		case XK_Shift_L: return KEY_SHIFT;
		default: return KEY_UNKNOWN;
	}
}

static KeyMods
get_x11_keymods(XKeyEvent event)
{
	return (KeyMods){
		.shift = event.state & ShiftMask,
		.control = event.state & ControlMask,
		.alt = event.state & Mod1Mask,
	};
}

static KeyEvent
xkeyevent_to_keyevent(XKeyEvent event)
{
	KeyEvent e = {0};
	e.type = EVENT_KEY_PRESS;
	KeySym keysym;
	int len = XLookupString(&event, e.text, sizeof(e.text) - 1, &keysym, NULL);
	e.text[len] = '\0';
	e.special_key = keycode_to_specialkey(keysym);
	e.mods = get_x11_keymods(event);
	return e;
}

static PlatformEvent
xevent_to_platform_event(XEvent xevent)
{
	switch (xevent.type) {
	case KeyPress:
		return (PlatformEvent){
			.keypress = xkeyevent_to_keyevent(xevent.xkey)
		};
	case ClientMessage:
		if ((Atom)xevent.xclient.data.l[0] == delete_window_atom) {
			return (PlatformEvent){.type = EVENT_QUIT};
		} else {
			return (PlatformEvent){.type = EVENT_NONE};
		}
	default:
		return (PlatformEvent){.type = EVENT_NONE};
	}
}

PlatformEvent
platform_next_event(void)
{
	while (XPending(display)) {
		XEvent xevent;
		XNextEvent(display, &xevent);
		PlatformEvent event = xevent_to_platform_event(xevent);
		if (event.type != EVENT_NONE) {
			return event;
		}
	}
	return (PlatformEvent){.type = EVENT_NONE};
}

void
swap_buffers(void)
{
	glXSwapBuffers(display, window);
}

void
get_window_size(int *width, int *height)
{
	XWindowAttributes gwa;
	XGetWindowAttributes(display, window, &gwa);
	*width = gwa.width;
	*height = gwa.height;
}

void
close_window(void)
{
	glXMakeCurrent(display, None, NULL);	
	glXDestroyContext(display, glc);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}
