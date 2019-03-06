#ifndef __IMGUI_IMPL_GLES2_TYPES_H_INCLUDED__
#define __IMGUI_IMPL_GLES2_TYPES_H_INCLUDED__

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

struct iig_handler_t
{
  GLuint shader_program;
  GLuint vertex_buffer;
  GLuint index_buffer;
  GLuint font_texture;
  unsigned int *width;
  unsigned int *height;
};

#endif /* __IMGUI_IMPL_GLES2_TYPES_H_INCLUDED__ */
