/*
 * common.h
 *
 *  Created on: 23-Mar-2019
 *      Author: tarun
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <GL/glew.h>
#include <GL/gl.h>

GLfloat Xrot = 180, Yrot = 0, Zrot = 0;
bool rect_ext_avail_ = false;
GLenum render_target_;
bool shader_ext_available_ = false;
GLint max_texture_passes_;
bool hw_conversion_avail_ = true;
GLenum pixel_format_;
GLenum pixel_data_type_;
GLuint i420_shader_;
unsigned buffer;
int width = 1280;
int height = 720;
int w_width = 848;
int w_height = 480;



#endif /* COMMON_H_ */
