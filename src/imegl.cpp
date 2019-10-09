#include "imgui_impl_egl.h"
#include "imgui_impl_gles2.h"

static iie_handler_t __iie_hdl = {};
static iig_handler_t __iig_hdl = {};

#ifdef X11
bool imegl_init(const char *window_name,
                const unsigned int width,
                const unsigned int height)
{
  if (iie_init(&__iie_hdl, window_name, width, height) == true)
    if (iig_init(&__iig_hdl, &(__iie_hdl.width), &(__iie_hdl.height)) == true)
      return true;
  return false;
}

bool imegl_x11_closed(void)
{
  return __iie_hdl.x_window_closed;
}
#else
bool imegl_init(void)
{
  if (iie_init(&__iie_hdl) == true)
    if (iig_init(&__iig_hdl, &(__iie_hdl.width), &(__iie_hdl.height)))
      return true;
  return false;
}
#endif  /* X11 or BRCM */

void imegl_shutdown(void)
{
  iig_shutdown(&__iig_hdl);
  iie_shutdown(&__iie_hdl);
}

void imegl_new_frame(void)
{
  iie_new_frame(&__iie_hdl);
  iig_new_frame(&__iig_hdl);
}

void imegl_render_draw_data(ImDrawData *draw_data)
{
  iig_render_draw_data(&__iig_hdl, (draw_data));
}

void imegl_swap_buffer(void)
{
  iie_swap_buffer(&__iie_hdl);
}
