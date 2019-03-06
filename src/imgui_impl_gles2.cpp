#include <stdlib.h>             // malloc
#include <imgui.h>

#include "imgui_impl_gles2_types.h"
#include "imegl_tools.h"

GLuint iig_load_shader(GLenum type, const char *shader_src)
{
  GLuint shader;
  GLint compiled;
  GLint info_len;
  char* info_log;

  shader = glCreateShader(type);

  if (shader == 0)
    return 0;

  glShaderSource(shader, 1, &shader_src, NULL);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

  if (compiled == 0)
    {
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);

      if (info_len > 1)
        {
          info_log = (char *) malloc(sizeof(char) * info_len);

          glGetShaderInfoLog(shader, info_len, NULL, info_log);
          log_err("Error compiling shader:\n%s", info_log);
          free(info_log);
        }
      glDeleteShader(shader);
      return 0;
    }

  return shader;
}

void iig_shutdown(iig_handler_t *iig_hdl)
{
  glDeleteProgram(iig_hdl->shader_program);
  glDeleteBuffers(1, &(iig_hdl->vertex_buffer));
  glDeleteBuffers(1, &(iig_hdl->index_buffer));
  glDeleteTextures(1, &(iig_hdl->font_texture));
}

bool iig_init(iig_handler_t *iig_hdl,
              unsigned int *width,
              unsigned int *height)
{
  GLchar vshader_str[] =
    "uniform vec2 i_dpy_size;"
    "attribute vec2 i_position;"
    "attribute vec2 i_texcoo;"
    "attribute vec4 i_color;"
    "varying vec2 v_texcoo;"
    "varying vec4 v_color;"
    "void main()"
    "{"
    "v_color = i_color;"
    "v_texcoo = i_texcoo;"
    "float x_coo;"
    "float y_coo;"
    "x_coo = (i_position.x *  2.0 / i_dpy_size.x) - 1.0;"
    "y_coo = (i_position.y * -2.0 / i_dpy_size.y) + 1.0;"
    "gl_Position = vec4(x_coo, y_coo, 0.0, 1.0);"
    "}";

  GLchar fshader_str[] =
    "precision mediump float;"
    "varying vec2 v_texcoo;"
    "varying vec4 v_color;"
    "uniform sampler2D i_texture;"
    "void main(){gl_FragColor = v_color * texture2D(i_texture, v_texcoo);}";

  GLuint vertex_shader;
  GLuint fragment_shader;
  GLint linked;
  GLint info_len;
  char* info_log;

  vertex_shader = iig_load_shader(GL_VERTEX_SHADER, vshader_str);
  if (vertex_shader == 0)
    {
      log_err("Unable to load vertex shader.");
      return false;
    }
  fragment_shader = iig_load_shader(GL_FRAGMENT_SHADER, fshader_str);
  if (fragment_shader == 0)
    {
      log_err("Unable to load fragment shader.");
      return false;
    }

  iig_hdl->shader_program = glCreateProgram();

  if (iig_hdl->shader_program == 0)
    return false;

  glAttachShader(iig_hdl->shader_program, vertex_shader);
  glAttachShader(iig_hdl->shader_program, fragment_shader);

  glBindAttribLocation(iig_hdl->shader_program, 0, "vPosition");

  glLinkProgram(iig_hdl->shader_program);

  glGetProgramiv(iig_hdl->shader_program, GL_LINK_STATUS, &linked);

  if (linked == 0)
    {
      glGetProgramiv(iig_hdl->shader_program,
                     GL_INFO_LOG_LENGTH,
                     &info_len);

      if (info_len > 1)
        {
          info_log = (char *) malloc(sizeof(char) * info_len);

          glGetProgramInfoLog(iig_hdl->shader_program,
                              info_len,
                              NULL,
                              info_log);
          log_err("Error linking program:\n%s", info_log);

          free(info_log);
        }
      glDeleteProgram(iig_hdl->shader_program);
      return false;
    }

  glUseProgram(iig_hdl->shader_program);

  glEnable(GL_TEXTURE_2D);

  glGenBuffers(1, &(iig_hdl->vertex_buffer));
  glGenBuffers(1, &(iig_hdl->index_buffer));

  iig_hdl->width  = width;
  iig_hdl->height = height;

  return true;
}

static inline void iig_font_init(iig_handler_t *iig_hdl)
{
  int font_width, font_height;
  unsigned char* font_pixels;
  ImGuiIO &io = ImGui::GetIO();

  io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height);
  glGenTextures(1, &(iig_hdl->font_texture));
  glBindTexture(GL_TEXTURE_2D, iig_hdl->font_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  // glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, 0);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA,
               font_width,
               font_height,
               0,
               GL_RGBA,
               GL_UNSIGNED_BYTE,
               font_pixels);
  io.Fonts->TexID = (ImTextureID)(intptr_t) iig_hdl->font_texture;
}

void iig_new_frame(iig_handler_t *iig_hdl)
{
  ImGuiIO &io = ImGui::GetIO();
  GLint var_pos;

  io.DisplaySize.x = *(iig_hdl->width);
  io.DisplaySize.y = *(iig_hdl->height);
  var_pos = glGetUniformLocation(iig_hdl->shader_program, "i_dpy_size");
  glUniform2f(var_pos,
              (GLfloat) *(iig_hdl->width),
              (GLfloat) *(iig_hdl->height));
  if (iig_hdl->font_texture == 0)
    iig_font_init(iig_hdl);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void iig_render_draw_data(iig_handler_t *iig_hdl, ImDrawData *draw_data)
{
  GLint var_pos;
  ImGuiIO& io = ImGui::GetIO();
  int fb_width =
    (int) (draw_data->DisplaySize.x * io.DisplayFramebufferScale.x);
  int fb_height =
    (int) (draw_data->DisplaySize.y * io.DisplayFramebufferScale.y);

  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glActiveTexture(GL_TEXTURE0);

  glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);

  glBindBuffer(GL_ARRAY_BUFFER, iig_hdl->vertex_buffer);

  var_pos = glGetAttribLocation(iig_hdl->shader_program, "i_position");
  glVertexAttribPointer(var_pos,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof (ImDrawVert),
                        (const void *) IM_OFFSETOF(ImDrawVert, pos));
  glEnableVertexAttribArray(var_pos);

  var_pos = glGetAttribLocation(iig_hdl->shader_program, "i_texcoo");
  glVertexAttribPointer(var_pos,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof (ImDrawVert),
                        (const void *) IM_OFFSETOF(ImDrawVert, uv));
  glEnableVertexAttribArray(var_pos);

  var_pos = glGetAttribLocation(iig_hdl->shader_program, "i_color");
  glVertexAttribPointer(var_pos,
                        4,
                        GL_UNSIGNED_BYTE,
                        GL_TRUE,
                        sizeof (ImDrawVert),
                        (const void *) IM_OFFSETOF(ImDrawVert, col));
  glEnableVertexAttribArray(var_pos);

  for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
      const ImDrawList* cmd_list = draw_data->CmdLists[n];
      const ImDrawIdx* idx_buffer_offset = 0;

      glBindBuffer(GL_ARRAY_BUFFER, iig_hdl->vertex_buffer);
      glBufferData(GL_ARRAY_BUFFER,
                   cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
                   cmd_list->VtxBuffer.Data,
                   GL_STREAM_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iig_hdl->index_buffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
                   cmd_list->IdxBuffer.Data,
                   GL_STREAM_DRAW);

      for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
          const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
          if (pcmd->UserCallback)
            pcmd->UserCallback(cmd_list, pcmd);
          else
            {
              glBindTexture(GL_TEXTURE_2D,
                            (GLuint) (intptr_t) pcmd->TextureId);
              glScissor((int)pcmd->ClipRect.x,
                        (int)(fb_height - pcmd->ClipRect.w),
                        (int)(pcmd->ClipRect.z - pcmd->ClipRect.x),
                        (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
              glDrawElements(GL_TRIANGLES,
                             pcmd->ElemCount,
                             GL_UNSIGNED_SHORT,
                             idx_buffer_offset);
            }
          idx_buffer_offset += pcmd->ElemCount;
        }
    }

}
