#ifndef __IMEGL_H_INCLUDED__
#define __IMEGL_H_INCLUDED__

#include <imgui.h>

#ifdef X11
bool imegl_init(const char *window_name,
                const unsigned int width,
                const unsigned int height);
bool imegl_x11_closed(void);
#else
bool imegl_init(void);
#endif  /* X11 or BRCM */

void imegl_shutdown(void);
void imegl_new_frame(void);
void imegl_render_draw_data(ImDrawData *draw_data);
void imegl_swap_buffer(void);

#endif  // __IMEGL_H_INCLUDED__
