#ifndef __IMGUI_IMPL_EGL_TYPES_H_INCLUDED__
#define __IMGUI_IMPL_EGL_TYPES_H_INCLUDED__

#include <EGL/egl.h>

#include "imgui_impl_linux_joystick.h"

#ifndef X11
#include <termios.h>

class KeyboardStdin
{
private:
  struct termios orig_termios;
  int orig_fl;
public:
  KeyboardStdin(void);
  ~KeyboardStdin(void);
  int GetKeyIn(void);
  void UpdateNav(void);
};
#endif

struct iie_handler_t
{
#ifdef X11
  Display *x_display;
  Atom close_window_atom;
  long x_event_mask;
  bool x_window_closed;
#else  // BRCM
  DISPMANX_DISPLAY_HANDLE_T dispman_display;
  DISPMANX_ELEMENT_HANDLE_T dispman_element;
  EGL_DISPMANX_WINDOW_T dispman_window;
#endif
  NativeWindowType native_window;
  EGLDisplay egl_display;
  EGLSurface egl_surface;
  EGLContext egl_context;
  unsigned int width;
  unsigned int height;
};

#endif  // __IMGUI_IMPL_EGL_TYPES_H_INCLUDED__
