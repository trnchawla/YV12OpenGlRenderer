/*
 * X11Window.h
 *
 *  Created on: 23-Mar-2019
 *      Author: Tarun
 */

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <string>

namespace window{
class X11Window{
	unsigned mWidth, mHeight;
	Display* mDisplay;
	Window mWindow;
	int mScreen = 0;
	std::string mTitle;
public:
	X11Window(int width, int height, std::string title):
		mWidth(width),
		mHeight(height),
		mTitle(title),
		mDisplay(nullptr){}

	bool CreateWindow();
	Display* GetDisplay(){
		return mDisplay;
	}
	Window GetWindow(){
		return mWindow;
	}
};
}

