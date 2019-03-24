/*
 * MainExecutable.cpp
 *
 *  Created on: 23-Mar-2019
 *      Author: Tarun
 */


#include "common.h"

#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <memory>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <assert.h>
#include <X11Window.h>


#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

using namespace std;

bool ARBExtensionAvailable(){
	char* ext = (char*) glGetString(GL_EXTENSIONS);
	std::string glExtensions;
	if (ext) {
		glExtensions.assign(ext);
	}
	if (glExtensions.find("GL_ARB_texture_rectangle") != std::string::npos
			|| glExtensions.find("GL_EXT_texture_rectangle")
			!= std::string::npos
			|| glExtensions.find("GL_NV_texture_rectangle")
			!= std::string::npos){
		return true;
	}
	return false;
}

static void DrawObject(void)
{

	glBegin(GL_QUADS);

	glTexCoord2f(0, 0);
	glVertex2f(-1.0, -1.0);

	glTexCoord2f(width, 0);
	glVertex2f(1.0, -1.0);

	glTexCoord2f(width , height);
	glVertex2f(1.0, 1.0);

	glTexCoord2f(0, height);
	glVertex2f(-1.0, 1.0);

	glEnd();
}

static void DisplayFrame()
{

	glClearColor(0.3, 0.3, 0.4, 1.0);
	glPushMatrix();
	glRotatef(Xrot, 1.0, 0.0, 0.0);
	glRotatef(Yrot, 0.0, 1.0, 0.0);
	glRotatef(Zrot, 0.0, 0.0, 1.0);
	DrawObject();
	glPopMatrix();

	glViewport( 0, 0, width, height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -15.0 );
}

bool CheckShader(int n_shader_object)
{
	int n_tmp;
	glGetShaderiv(n_shader_object, GL_COMPILE_STATUS, &n_tmp);
	bool b_compiled = n_tmp == GL_TRUE;
	int n_log_length;
	glGetShaderiv(n_shader_object, GL_INFO_LOG_LENGTH, &n_log_length);
	// query status ...

	if(n_log_length > 1) {
		char *p_s_temp_info_log;
		if(!(p_s_temp_info_log = (char*)malloc(n_log_length)))
			return false;
		int n_tmp;
		glGetShaderInfoLog(n_shader_object, n_log_length, &n_tmp,
				p_s_temp_info_log);
		assert(n_tmp <= n_log_length);

		fprintf(stderr, "%s\n", p_s_temp_info_log);
		free(p_s_temp_info_log);
	}
	// get/concat info-log

	return b_compiled;
}
void CreateShader(){
	static const char *p_s_vertex_shader =
			"varying vec2 t;"
			"void main()"
			"{"
			"    t = gl_MultiTexCoord0.xy;"
			"    gl_Position = ftransform();"
			"}";
	static const char *p_s_fragment_shader =
			"#extension GL_ARB_texture_rectangle : enable\n"
			"varying vec2 t;"
			"uniform sampler2DRect tex;"
			"void main()"
			"{"
			"    vec2 tcEven = vec2(floor(t.x * .5) * 2.0, t.y);"
			"    vec2 tcOdd = vec2(tcEven.x + 1.0, t.y);"
			"    float Cb = texture2DRect(tex, tcEven).x - .5;"
			"    float Cr = texture2DRect(tex, tcOdd).x - .5;"
			"    float y = texture2DRect(tex, t).w;" // redundant texture read optimized away by texture cache
			"    float r = y + 1.28033 * Cr;"
			"    float g = y - .21482 * Cb - .38059 * Cr;"
			"    float b = y + 2.12798 * Cb;"
			"    gl_FragColor = vec4(r, g, b, 1.0);"
			"}";
	glewInit();
	int v = glCreateShader(GL_VERTEX_SHADER);
	int f = glCreateShader(GL_FRAGMENT_SHADER);
	int p = glCreateProgram();
	glShaderSource(v, 1, &p_s_vertex_shader, 0);
	glShaderSource(f, 1, &p_s_fragment_shader, 0);
	glCompileShader(v);
	CheckShader(v);
	glCompileShader(f);
	CheckShader(f);
	glAttachShader(p, v);
	glAttachShader(p, f);
	glLinkProgram(p);
	glUseProgram(p);
	glUniform1i(glGetUniformLocation(p, "tex"), 0);
}

int main(){
	window::X11Window window(w_width, w_height, "Renderer");
	window.CreateWindow();
	int screen = DefaultScreen(window.GetDisplay());
	int attr_list[] = { GLX_DOUBLEBUFFER, GLX_RGBA, GLX_RED_SIZE, 4,
			GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4, GLX_DEPTH_SIZE, 16,
			None, };
	XVisualInfo* vi;
	vi = glXChooseVisual(window.GetDisplay(), screen, attr_list);
	GLXContext context_;
	context_ = glXCreateContext(window.GetDisplay(), vi, 0, true);
	if (context_ == NULL) {
		cout<< "glXCreateContext failed";
		return -1;
	}

	if (!glXMakeCurrent(window.GetDisplay(), window.GetWindow(), context_)) {
		cout<<"glXMakeCurrent failed";
		return -1;
	}

	auto buffer_size = width * height * 3/2;
	auto mem = std::make_unique<char[]>(buffer_size);
	XEvent event;
	while (XPending(window.GetDisplay())) {
		XNextEvent(window.GetDisplay(), &event);
		switch (event.type) {
		case ConfigureNotify:
			glViewport(0,0,event.xconfigure.width,
					event.xconfigure.height);
			break;
		default:
			break;
		}
	}
	unsigned char* buffer_ = (unsigned char*)mem.get();
	FILE* file = nullptr;
	std::string input_file_location = STRINGIFY(RES_LOC);
	input_file_location += "/image.yuv";
	cout<<input_file_location<<endl;
	file = fopen(input_file_location.c_str(),"rb");
	if(file == nullptr){
		cout<<"Not able to open file\n";
	}else{
		cout<<"Able to open file size = "<<buffer_size<<endl;
		int read_bytes = fread(buffer_,1 , buffer_size, file);
		cout<<"read_bytes are "<<read_bytes<<"\n";
	}
	fclose(file);

	if(!ARBExtensionAvailable()){
		cout<<"ARB extension not available"<<endl;
		return -1;
	}

#ifdef LINEAR_FILTER
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#else
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
	CreateShader();

	GLushort* ImageYUV = nullptr;
	unsigned yuv_file_size = width * height * 3/2;
	ImageYUV = new GLushort[width * height];
	int chromaWidth = width / 2;
	int chromaHeight = height / 2; // 2x2 luminance subsampling
	const GLubyte *pCb = buffer_ + width * height; // Cb block after Y
	const GLubyte *pCr = pCb + chromaWidth * chromaHeight; // Cr block after Cb

	for(int i = 0; i < width * height ; ++ i) {
		int x = i % width;
		int y = i / width;
		GLubyte cb = pCb[(x / 2) + (y / 2) * chromaWidth];
		GLubyte cr = pCr[(x / 2) + (y / 2) * chromaWidth];
		ImageYUV[i] = (buffer_[i] << 8) | ((x & 1)? cr : cb);
	}

	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0,
			GL_LUMINANCE_ALPHA, width, height, 0,
			GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, ImageYUV);

	assert(glGetError() == GL_NO_ERROR);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0,
			0, 0, width, height,
			GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, ImageYUV);

	assert(glGetError() == GL_NO_ERROR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_RECTANGLE_ARB);

	glShadeModel(GL_FLAT);

	glClear( GL_COLOR_BUFFER_BIT );
	DisplayFrame();
	printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
	printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
	printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
	printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));
	while(1)
	{
		while (XPending(window.GetDisplay())) {
			XNextEvent(window.GetDisplay(), &event);
			switch (event.type) {
			case ConfigureNotify:
				glViewport(0,0,event.xconfigure.width,
						event.xconfigure.height);
				break;
			default:
				break;
			}
		}
		DrawObject();
		glXSwapBuffers(window.GetDisplay(), window.GetWindow());
		sleep(1);
	}

}

