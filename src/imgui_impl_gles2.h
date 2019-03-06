#ifndef __IMGUI_IMPL_GLES2_H_INCLUDED__
#define __IMGUI_IMPL_GLES2_H_INCLUDED__

#include "imgui_impl_gles2_types.h"
#include <imgui.h>

void iig_shutdown(iig_handler_t *iig_hdl);
bool iig_init(iig_handler_t *iig_hdl,
              unsigned int *width,
              unsigned int *height);
void iig_new_frame(iig_handler_t *iig_hdl);
void iig_render_draw_data(iig_handler_t *iig_hdl, ImDrawData *draw_data);

#endif /* __IMGUI_IMPL_GLES2_H_INCLUDED__ */
