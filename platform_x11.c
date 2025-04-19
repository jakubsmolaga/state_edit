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
	window = XCreateWindow(display, root, 0, 0, 600, 600, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	XMapWindow(display, window);
	XStoreName(display, window, title);
	glc = glXCreateContext(display, vi, NULL, GL_TRUE);
	glXMakeCurrent(display, window, glc);
	delete_window_atom = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &delete_window_atom, 1);
}

int
get_next_frame(void)
{
	int running = 1;
	while (XPending(display)) {
		XEvent event;
		XNextEvent(display, &event);
		if (event.type == ClientMessage && event.xclient.data.l[0] == delete_window_atom) {
			running = 0;
			break;
		}
		// TODO: handle all events
	}
	return running;
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
