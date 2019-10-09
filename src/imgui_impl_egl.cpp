#include <unistd.h>             // usleep

#include <imgui.h>

#include "imgui_impl_egl_types.h"
#include "imegl_tools.h"

#ifndef X11

extern "C"
{
#include <bcm_host.h>
};

#include <fcntl.h>
KeyboardStdin::KeyboardStdin(void)
{
  if (isatty(STDIN_FILENO))
    {
      struct termios new_termios;

      tcgetattr(STDIN_FILENO, &orig_termios);

      new_termios = orig_termios;
      new_termios.c_lflag &= ~(ICANON | ECHO | ECHOCTL | ECHONL);
      new_termios.c_cflag |= HUPCL;
      new_termios.c_cc[VMIN] = 0;
      // new_termios.c_cc[VTIME] = 0;

      tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    }
  else
    {
      orig_fl = fcntl(STDIN_FILENO, F_GETFL);
      fcntl(STDIN_FILENO, F_SETFL, orig_fl | O_NONBLOCK);
    }
}

KeyboardStdin::~KeyboardStdin(void)
{
  if (isatty(STDIN_FILENO))
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
  else
    fcntl(STDIN_FILENO, F_SETFL, orig_fl);
}

#define KBDSTDIN_MAXCHAR 8
int KeyboardStdin::GetKeyIn(void)
{
  int ch[KBDSTDIN_MAXCHAR];
  size_t idx;

  for (idx = 0; idx < KBDSTDIN_MAXCHAR; idx++)
    {
      ch[idx] = getc(stdin);
      if (ch[idx] == EOF)
        break;
    }

  if (ch[0] == '\033' && ch[1] == '[')
    {
      ch[0] = (ch[0] << 16) + (ch[1] << 8) + ch[2];
    }
  else if (idx != 1)
    ch[idx] = EOF;

  clearerr(stdin);

  return ch[0];
}
#undef KBDSTDIN_MAXCHAR

void KeyboardStdin::UpdateNav(void)
{
  int keyin;
  ImGuiIO &io = ImGui::GetIO();

  for (keyin = GetKeyIn();
       keyin != EOF;
       keyin = GetKeyIn())
    {
      // log_msg("New one key: %08X\n", keyin);
      switch (keyin)
        {
        case 0x1b5b44:
          io.NavInputs[ImGuiNavInput_DpadLeft] = 1.0f;
          break;
        case 0x1b5b41:
          io.NavInputs[ImGuiNavInput_DpadUp] = 1.0f;
          break;
        case 0x1b5b43:
          io.NavInputs[ImGuiNavInput_DpadRight] = 1.0f;
          break;
        case 0x1b5b42:
          io.NavInputs[ImGuiNavInput_DpadDown] = 1.0f;
          break;
        case 0x7f:              // Delete
        case 0x1b:              // Esc
          io.NavInputs[ImGuiNavInput_Cancel] = 1.0f;
          break;
        case 0xa:               // Return
          io.NavInputs[ImGuiNavInput_Activate] = 1.0f;
          break;
        case 0x9:               // Tab
          io.NavInputs[ImGuiNavInput_KeyMenu_] = 1.0f;
          break;
        }
    }
}

#endif  // NOT X11

#define EGL_ERROR_STR(err_num)                                          \
  (err_num) == EGL_SUCCESS ? "The last function succeeded without error." : \
    (err_num) == EGL_NOT_INITIALIZED ? "EGL is not initialized, or could not" \
    " be initialized, for the specified EGL display connection." :      \
    (err_num) == EGL_BAD_ACCESS ? "EGL cannot access a requested resource " \
    "(for example a context is bound in another thread)." :             \
    (err_num) == EGL_BAD_ALLOC ? "EGL failed to allocate resources for the" \
    " requested operation." :                                           \
    (err_num) == EGL_BAD_ATTRIBUTE ? "An unrecognized attribute or attribute" \
    " value was passed in the attribute list." :                        \
    (err_num) == EGL_BAD_CONTEXT ? "An EGLContext argument does not name a" \
    " valid EGL rendering context." :                                   \
    (err_num) == EGL_BAD_CONFIG ? "An EGLConfig argument does not name a" \
    " valid EGL frame buffer configuration." :                          \
    (err_num) == EGL_BAD_CURRENT_SURFACE ? "The current surface of the" \
    " calling thread is a window, pixel buffer or pixmap that is no longer" \
    " valid." :                                                         \
    (err_num) == EGL_BAD_DISPLAY ? "An EGLDisplay argument does not name" \
    " a valid EGL display connection." :                                \
    (err_num) == EGL_BAD_SURFACE ? "An EGLSurface argument does not name a" \
    " valid surface (window, pixel buffer or pixmap) configured for GL" \
    " rendering." :                                                     \
    (err_num) == EGL_BAD_MATCH ? "Arguments are inconsistent (for example," \
    " a valid context requires buffers not supplied by a valid surface)." : \
    (err_num) == EGL_BAD_PARAMETER ? "One or more argument values are"  \
    " invalid." :                                                       \
    (err_num) == EGL_BAD_NATIVE_PIXMAP ? "A NativePixmapType argument does" \
    " not refer to a valid native pixmap." :                            \
    (err_num) == EGL_BAD_NATIVE_WINDOW ? "A NativeWindowType argument does" \
    " not refer to a valid native window." :                            \
    (err_num) == EGL_CONTEXT_LOST ? "A power management event has occurred." \
    " The application must destroy all contexts and reinitialise OpenGL ES" \
    " state and objects to continue rendering." :                       \
    "Unknown Error."

EGLBoolean dump_egl_error(const char *error_on_str)
{
  EGLint error;

  error = eglGetError();
  if (error == EGL_SUCCESS)
    return EGL_FALSE;
  if (error_on_str != NULL)
    log_err("Error on %s:\n%s\n", error_on_str, EGL_ERROR_STR(error));
  else
    log_err("Error:\n%s\n", EGL_ERROR_STR(error));

  return EGL_TRUE;
}

#ifndef X11
#define DEFAULT_BRCM_DEVICE 0
#define DEFAULT_BRCM_PRIORITY 0
#endif

void iie_shutdown(iie_handler_t *iie_hdl)
{
#ifndef X11
  DISPMANX_UPDATE_HANDLE_T dispman_update;
#endif

  eglMakeCurrent(iie_hdl->egl_display,
                 EGL_NO_SURFACE,
                 EGL_NO_SURFACE,
                 EGL_NO_CONTEXT);
  eglDestroyContext(iie_hdl->egl_display, iie_hdl->egl_context);
  eglDestroySurface(iie_hdl->egl_display, iie_hdl->egl_surface);
  eglTerminate(iie_hdl->egl_display);
#ifdef X11
  XDestroyWindow(iie_hdl->x_display, iie_hdl->native_window);
  XCloseDisplay(iie_hdl->x_display);
#else  // BRCM
  dispman_update = vc_dispmanx_update_start(DEFAULT_BRCM_PRIORITY);
  vc_dispmanx_element_remove(dispman_update, iie_hdl->dispman_element);
  vc_dispmanx_update_submit_sync(dispman_update);
  vc_dispmanx_display_close(iie_hdl->dispman_display);
#endif
}

#define CONFIG_NUM 1

static inline bool _iie_egl_init(iie_handler_t *iie_hdl)
{
  EGLConfig egl_config[CONFIG_NUM] = {};
  EGLint num_config;
  const EGLint attribute_list[] = {
    EGL_RED_SIZE,        8,
    EGL_GREEN_SIZE,      8,
    EGL_BLUE_SIZE,       8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
    EGL_NONE
  };

  if (eglInitialize(iie_hdl->egl_display, NULL, NULL) == EGL_FALSE)
    {
      dump_egl_error("eglInitialize");
      return false;
    }

  if (eglChooseConfig(iie_hdl->egl_display,
                      attribute_list,
                      egl_config,
                      CONFIG_NUM,
                      &num_config)
      == EGL_FALSE)
    {
      dump_egl_error("eglChooseConfig");
      return false;
    }

  const EGLint ctx_attrib[] = {EGL_CONTEXT_CLIENT_VERSION, 2,
                               EGL_NONE};
  iie_hdl->egl_context = eglCreateContext(iie_hdl->egl_display,
                                          egl_config[0],
                                          EGL_NO_CONTEXT,
                                          ctx_attrib);
  if (dump_egl_error("eglCreateContext") == EGL_TRUE)
    return false;

  iie_hdl->egl_surface = eglCreateWindowSurface(iie_hdl->egl_display,
                                                egl_config[0],
                                                iie_hdl->native_window,
                                                NULL);
  if (dump_egl_error("eglCreateWindowSurface") == EGL_TRUE)
    return false;

  if (eglMakeCurrent(iie_hdl->egl_display,
                     iie_hdl->egl_surface,
                     iie_hdl->egl_surface,
                     iie_hdl->egl_context) == EGL_FALSE)
    {
      dump_egl_error("eglMakeCurrent");
      return false;
    }

  usleep(100000);   // Why ??? to remove as soon as possible
  // Problem on X11 platform without the usleep (to identify)
  // -> Sometimes nothing is displayed

  return true;
}

static LinuxJoystickNav __joy_nav;

bool iie_joystick_available(void) { return __joy_nav.IsAvailable(); }
bool iie_joystick_calibrated(void) { return __joy_nav.IsCalibrated(); }
bool iie_calibrate_joystick(void) { return __joy_nav.Calibrate(); }

#ifdef X11

bool iie_init(iie_handler_t *iie_hdl,
              const char *window_name,
              const unsigned int width,
              const unsigned int height)
{
  Window root_window;

  iie_hdl->width  = width;
  iie_hdl->height = height;
  iie_hdl->x_display = XOpenDisplay(NULL);
  root_window = DefaultRootWindow(iie_hdl->x_display);
  iie_hdl->native_window = XCreateSimpleWindow(iie_hdl->x_display, root_window,
                                               0, 0,
                                               width, height,
                                               0, 0, 0);
  iie_hdl->x_window_closed = false;
  iie_hdl->x_event_mask = ResizeRedirectMask
    | ButtonPressMask
    | ButtonReleaseMask
    | KeyPressMask
    | KeyReleaseMask;
  XSelectInput(iie_hdl->x_display,
               iie_hdl->native_window,
               iie_hdl->x_event_mask);

  // XWMHints *hints = XAllocWMHints();
  // hints->input = True;
  // hints->flags = InputHint;
  // XSetWMHints(iie_hdl->x_display, iie_hdl->native_window, hints);
  // XFree(hints);

  // Prevent any resize
  XSizeHints *size_hints = XAllocSizeHints();
  size_hints->flags = PMinSize | PMaxSize;
  size_hints->min_width  = size_hints->max_width  = width;
  size_hints->min_height = size_hints->max_height = height;
  XSetWMNormalHints(iie_hdl->x_display, iie_hdl->native_window, size_hints);
  XFree(size_hints);

  XStoreName(iie_hdl->x_display, iie_hdl->native_window, window_name);
  XMapWindow(iie_hdl->x_display, iie_hdl->native_window);

  iie_hdl->close_window_atom = XInternAtom(iie_hdl->x_display,
                                           "WM_DELETE_WINDOW",
                                           False);
  XSetWMProtocols(iie_hdl->x_display,
                  iie_hdl->native_window,
                  &(iie_hdl->close_window_atom),
                  1);

  iie_hdl->egl_display = eglGetDisplay(iie_hdl->x_display);

  if (iie_hdl->egl_display == EGL_NO_DISPLAY)
    {
      log_err("Got no EGL display.\n");
      return false;
    }

  if (_iie_egl_init(iie_hdl) == false)
    return false;

  return true;
}

#define IIE_XEV_TYPE_TO_STR(type)                               \
  ((type) == GraphicsExpose) ? "Graphics expose event" :        \
  ((type) == NoExpose)       ? "No expose event" :              \
  ((type) == Expose)         ? "Expose event" :                 \
  ((type) == ButtonPress)    ? "Button press" :                 \
  ((type) == ButtonRelease)  ? "Button release" :               \
  ((type) == KeyPress)       ? "Key press" :                    \
  ((type) == KeyRelease)     ? "Key release" :                  \
  ((type) == ResizeRequest)  ? "Resize event" :                 \
  ((type) == ClientMessage)  ? "Client message event" :         \
  "Unknown"

#define keysym_to_ImGuiNavInput(xkeysym)                        \
  ((xkeysym) == XK_Left      ? ImGuiNavInput_DpadLeft :         \
   (xkeysym) == XK_Up        ? ImGuiNavInput_DpadUp :           \
   (xkeysym) == XK_Right     ? ImGuiNavInput_DpadRight :        \
   (xkeysym) == XK_Down      ? ImGuiNavInput_DpadDown :         \
   (xkeysym) == XK_Escape    ? ImGuiNavInput_Cancel :           \
   (xkeysym) == XK_Return    ? ImGuiNavInput_Activate :         \
   (xkeysym) == XK_BackSpace ? ImGuiNavInput_Cancel :           \
   (xkeysym) == XK_Tab       ? ImGuiNavInput_KeyMenu_ :         \
   (xkeysym) == XK_space     ? ImGuiNavInput_Activate :         \
   -1)

void iie_new_frame(iie_handler_t *iie_hdl)
{
  XEvent xev;
  size_t button_idx;
  KeySym keysym;
  ImGuiIO &io = ImGui::GetIO();

  memset(io.NavInputs, 0, sizeof(io.NavInputs));

  while (XPending(iie_hdl->x_display) != 0)
    {
      XNextEvent(iie_hdl->x_display, &xev);
      // log_msg("Event type: %s\n", IIE_XEV_TYPE_TO_STR(xev.type));
      switch (xev.type)
        {
        case ButtonPress:
          button_idx = xev.xbutton.button - 1;
          io.MouseDown[button_idx] = true;
          io.MousePos = ImVec2(xev.xbutton.x, xev.xbutton.y);
          break;
        case ButtonRelease:
          button_idx = xev.xbutton.button - 1;
          io.MouseDown[button_idx] = false;
          io.MousePos = ImVec2(xev.xbutton.x, xev.xbutton.y);
          break;
        case KeyPress:
          keysym = XLookupKeysym(&(xev.xkey), 0);
          io.NavInputs[keysym_to_ImGuiNavInput(keysym)] = 1.0f;
          break;
        // case KeyRelease:
        //   log_msg("key release code% 4d\n", xev.xkey.keycode);
        //   keysym = XLookupKeysym(&(xev.xkey), 0);
        //   io.KeysDown[ImGuiKey_from_keysym(keysym)] = 1.0f;
        //   break;
        case ClientMessage:
          if ((Atom) xev.xclient.data.l[0] == iie_hdl->close_window_atom)
            iie_hdl->x_window_closed = true;
          break;
        case ResizeRequest:
          iie_hdl->width = xev.xresizerequest.width;
          iie_hdl->height = xev.xresizerequest.height;
          break;
        }
    }

  __joy_nav.UpdateNav();
}

#else  // BRCM

bool iie_init(iie_handler_t *iie_hdl)
{
  DISPMANX_UPDATE_HANDLE_T dispman_update;
  VC_RECT_T dst_rect, src_rect;

  bcm_host_init();
  iie_hdl->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  if (iie_hdl->egl_display == EGL_NO_DISPLAY)
    {
      log_err("Got no EGL display.\n");
      return false;
    }

  if (graphics_get_display_size(DEFAULT_BRCM_DEVICE,
                                &(iie_hdl->width),
                                &(iie_hdl->height)) < 0)
    {
      log_err("Unable to get display size. (from device %d)\n",
              DEFAULT_BRCM_DEVICE);
      return false;
    }

  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.width  = iie_hdl->width;
  dst_rect.height = iie_hdl->height;

  src_rect.x = 0;
  src_rect.y = 0;
  src_rect.width  = iie_hdl->width << 16;
  src_rect.height = iie_hdl->height << 16;

  iie_hdl->dispman_display = vc_dispmanx_display_open(DEFAULT_BRCM_DEVICE);
  dispman_update = vc_dispmanx_update_start(DEFAULT_BRCM_PRIORITY);

  iie_hdl->dispman_element =
    vc_dispmanx_element_add(dispman_update,
                            iie_hdl->dispman_display,
                            0, // layer
                            &dst_rect,
                            0, // src
                            &src_rect,
                            DISPMANX_PROTECTION_NONE,
                            0,  // alpha
                            0,  // clamp
                            (DISPMANX_TRANSFORM_T) 0); // transform

  iie_hdl->dispman_window.element = iie_hdl->dispman_element;
  iie_hdl->dispman_window.width  = iie_hdl->width;
  iie_hdl->dispman_window.height = iie_hdl->height;
  iie_hdl->native_window = &(iie_hdl->dispman_window);

  vc_dispmanx_update_submit_sync(dispman_update);

  if (_iie_egl_init(iie_hdl) == false)
    return false;

  return true;
}

static KeyboardStdin __kbd_stdin;

void iie_new_frame(iie_handler_t *iie_hdl)
{
  ImGuiIO &io = ImGui::GetIO();

  memset(io.NavInputs, 0, sizeof(io.NavInputs));

  __kbd_stdin.UpdateNav();
  __joy_nav.UpdateNav();
}

#endif  // X11 or BRCM

bool iie_swap_buffer(iie_handler_t *iie_hdl)
{
  if (eglSwapBuffers(iie_hdl->egl_display, iie_hdl->egl_surface) == EGL_FALSE)
    {
      dump_egl_error("eglSwapBuffers");
      return false;
    }

  return true;
}
