#ifndef __IMGUI_IMPL_EGL_H_INCLUDED__
#define __IMGUI_IMPL_EGL_H_INCLUDED__

#include "imgui_impl_egl_types.h"

void iie_shutdown(iie_handler_t *hdl);

#ifdef X11
bool iie_init(iie_handler_t *hdl,
              const char *window_name,
              const unsigned int width,
              const unsigned int height);
#else // BRCM
bool iie_init(iie_handler_t *hdl);
#endif

bool iie_swap_buffer(iie_handler_t *iie_hdl);
void iie_new_frame(iie_handler_t *iie_hdl);

bool iie_joystick_available(void);
bool iie_joystick_calibrated(void);
bool iie_calibrate_joystick(void);

#endif  // __IMGUI_IMPL_EGL_H_INCLUDED__
