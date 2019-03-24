/*
 * X11Window.cpp
 *
 *  Created on: 23-Mar-2019
 *      Author: Tarun
 */


#include <X11Window.h>
#include <GL/glx.h>

namespace window{
bool X11Window::CreateWindow(){
	if((mDisplay = XOpenDisplay(nullptr)) == nullptr){
		return false;
	}
	mScreen = DefaultScreen(mDisplay);
	int attr_list[] = { GLX_DOUBLEBUFFER, GLX_RGBA, GLX_RED_SIZE, 4,
			GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4, GLX_DEPTH_SIZE, 16,
			None, };
	XVisualInfo* vi;
	vi = glXChooseVisual(mDisplay, mScreen, attr_list);
	XSetWindowAttributes window_attributes;
	window_attributes.colormap = XCreateColormap(
			mDisplay, RootWindow(mDisplay, vi->screen), vi->visual, AllocNone);
	window_attributes.border_pixel = 0;
	window_attributes.event_mask = StructureNotifyMask | ExposureMask;
	mWindow = XCreateWindow(mDisplay, RootWindow(mDisplay, vi->screen), 0, 0,
			mWidth, mHeight, 0, vi->depth, InputOutput,
			vi->visual, CWBorderPixel | CWColormap | CWEventMask,
			&window_attributes);
    XSetStandardProperties(mDisplay, mWindow, mTitle.c_str(), mTitle.c_str(), None,
            nullptr, 0, nullptr);
    XMapRaised(mDisplay, mWindow);
    XFree(vi);
    Atom wm_delete = XInternAtom(mDisplay, "WM_DELETE_WINDOW", True);
    if (wm_delete != None) {
        XSetWMProtocols(mDisplay, mWindow, &wm_delete, 1);
    }
	return true;
}
}
