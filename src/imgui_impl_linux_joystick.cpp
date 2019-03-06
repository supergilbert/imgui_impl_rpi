#include <sys/types.h>          // stat
#include <sys/stat.h>           // stat
#include <unistd.h>             // stat
#include <fcntl.h>              // open
#include <stdlib.h>             // getenv
#include <string.h>             // strlen
#include <linux/joystick.h>
#include <imgui.h>
#include <stdio.h>              // snprintf

#include "imgui_impl_linux_joystick.h"
#include "imgui_impl_egl.h"
#include "imegl.h"
// #include "imegl.h"

#define LJ_CALIBRATION_DIR_SUF ".config/imegl_linux_joystick"
#define LJ_DIR_LEN 64
#define LJ_CALIBRATION_PATH_LEN (LJ_DIR_LEN + LJ_PATH_MAX_LEN + 4)

bool is_directory(const char *path)
{
  struct stat statbuf;

  if (stat(path, &statbuf) != 0)
    return false;
  if (S_ISDIR(statbuf.st_mode))
    return true;
  return false;
}

bool LinuxJoystickNav::LoadCalibrationFromPath(const char *calibration_path)
{
  int fd = open(calibration_path, O_RDONLY);

  if (fd == -1)
    return false;
  read(fd, min_thresh,       sizeof (float));
  read(fd, max_thresh,       sizeof (float));
  read(fd, &(min_thresh[1]), sizeof (float));
  read(fd, &(max_thresh[1]), sizeof (float));
  close(fd);

  return true;
}

bool LinuxJoystickNav::LoadCalibration(void)
{
  char calibration_dir[LJ_DIR_LEN];
  char calibration_path[LJ_CALIBRATION_PATH_LEN];

  snprintf(calibration_dir, LJ_DIR_LEN,
           "%s/" LJ_CALIBRATION_DIR_SUF,
           getenv("HOME"));
  if (is_directory(calibration_dir) == false)
    return false;

  snprintf(calibration_path, LJ_CALIBRATION_PATH_LEN,
           "%s/%s.cal",
           calibration_dir,
           joystick_name);
  if (access(calibration_path, R_OK) == -1)
    return false;

  return LoadCalibrationFromPath(calibration_path);
}

bool LinuxJoystickNav::SaveCalibrationToPath(const char *calibration_path)
{
  int fd;

  fd = open(calibration_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1)
    return false;

  write(fd, min_thresh,       sizeof (float));
  write(fd, max_thresh,       sizeof (float));
  write(fd, &(min_thresh[1]), sizeof (float));
  write(fd, &(max_thresh[1]), sizeof (float));
  close(fd);

  return true;
}

bool mkdir_recurs(const char *dir)
{
  char tmp[256];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp),"%s",dir);
  len = strlen(tmp);
  if(tmp[len - 1] == '/')
    tmp[len - 1] = 0;
  for(p = tmp + 1; *p; p++)
    if(*p == '/')
      {
        *p = 0;
        if (is_directory(tmp) == false)
          if (mkdir(tmp, S_IRWXU) == -1)
            return false;
        *p = '/';
      }
  if (mkdir(tmp, S_IRWXU) == -1)
    return false;
  return true;
}

bool LinuxJoystickNav::SaveCalibration(void)
{
  char calibration_dir[LJ_DIR_LEN];
  char calibration_path[LJ_CALIBRATION_PATH_LEN];

  snprintf(calibration_dir, LJ_DIR_LEN,
           "%s/" LJ_CALIBRATION_DIR_SUF,
           getenv("HOME"));
  if (is_directory(calibration_dir) == false)
    if (mkdir_recurs(calibration_dir) == false)
      return false;

  snprintf(calibration_path, LJ_CALIBRATION_PATH_LEN,
           "%s/%s.cal",
           calibration_dir,
           joystick_name);

  return SaveCalibrationToPath(calibration_path);
}

bool LinuxJoystickNav::Load(const char *dev_path)
{
  if (joystick_fd != 0 && joystick_fd != -1)
    close(joystick_fd);

  joystick_fd = open(dev_path, O_RDONLY|O_NONBLOCK);

  calibrated = available = false;

  if (joystick_fd == -1)
    return false;

  if (ioctl(joystick_fd, JSIOCGNAME(LJ_NAME_MAX_LEN), joystick_name) < 0)
    return false;

  available = true;

  calibrated = LoadCalibration();

  return true;
}

LinuxJoystickNav::LinuxJoystickNav(const char *dev_path)
{
  Load(dev_path);
}

LinuxJoystickNav::LinuxJoystickNav(void)
{
  Load("/dev/input/js0");
}

LinuxJoystickNav::~LinuxJoystickNav(void)
{
  close(joystick_fd);
}

#define THRESHOLD 20             /* in percent */
static void _set_joystick_threshold(__s16 min,
                                    __s16 max,
                                    float *min_thresh,
                                    float *max_thresh)
{
  float threshold_val = (max - min) * THRESHOLD / 100;

  *min_thresh = min + threshold_val;
  *max_thresh = max - threshold_val;
}

bool LinuxJoystickNav::Calibrate(void)
{
  bool button1_pressed = false, first_value_found[2] = {false, false};
  static __s16 min[2], max[2];
  struct js_event event;
  bool bool_var = true;
  ImGuiIO &io = ImGui::GetIO();

  if (available == false)
    return false;

  lseek(joystick_fd, 0, SEEK_END);

  while (button1_pressed == false)
    {
      while (read(joystick_fd, &event, sizeof (struct js_event)) > 0)
        {
          switch (event.type)
            {
            case JS_EVENT_AXIS:
              if (event.number > 1)
                continue;
              if (first_value_found[event.number] == false)
                {
                  min[event.number] = max[event.number] = event.value;
                  first_value_found[event.number] = true;
                }
              else
                {
                  if (event.value < min[event.number])
                    {
                      min[event.number] = event.value;
                      _set_joystick_threshold(min[event.number],
                                              max[event.number],
                                              &(min_thresh[event.number]),
                                              &(max_thresh[event.number]));
                    }
                  else if (event.value > max[event.number])
                    {
                      max[event.number] = event.value;
                      _set_joystick_threshold(min[event.number],
                                              max[event.number],
                                              &(min_thresh[event.number]),
                                              &(max_thresh[event.number]));
                    }
                }
              break;
            case JS_EVENT_BUTTON:
              if (event.value == 1)    /* On button release */
                if (event.number == 0) /* button 1 */
                  button1_pressed = true;
            }
        }
      imegl_new_frame();
      ImGui::NewFrame();
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(io.DisplaySize);
      ImGui::SetNextWindowBgAlpha(1.0f);
      ImGui::Begin("Calibration",
                   &bool_var,
                   ImGuiWindowFlags_NoTitleBar
                   | ImGuiWindowFlags_NoDecoration
                   | ImGuiWindowFlags_NoMove);

      ImGui::Text("Calibration state:\n"
                  "Move Axis 0 and 1 to his edge, then press button 1.\n"
                  "  Axis0 min:%hd max:%hd min_thresh:%f max_thresh:%f\n"
                  "  Axis1 min:%hd max:%hd min_thresh:%f max_thresh:%f\n",
                  min[0], max[0], min_thresh[0], max_thresh[0],
                  min[1], max[1], min_thresh[1], max_thresh[1]);

      ImGui::End();
      ImGui::Render();
      imegl_render_draw_data(ImGui::GetDrawData());
      imegl_swap_buffer();

      fflush(stdout);
      usleep(200000);
    }
  SaveCalibration();
  calibrated = true;
  return true;
}

bool LinuxJoystickNav::IsCalibrated(void)
{
  return calibrated;
}

bool LinuxJoystickNav::IsAvailable(void)
{
  return available;
}

void LinuxJoystickNav::UpdateNav(void)
{
  struct js_event event;
  static __s16 last_val[2];
  ImGuiIO &io = ImGui::GetIO();

  if (available == false)
    return;

  while (read(joystick_fd, &event, sizeof (struct js_event)) > 0)
    {
      switch (event.type)
        {
        case JS_EVENT_AXIS:
          if (calibrated == true)
            {
              if (event.value < min_thresh[event.number]
                  && last_val[event.number] >= min_thresh[event.number])
                {
                  if (event.number == 0)
                    io.NavInputs[ImGuiNavInput_DpadLeft] = 1.0f;
                  else if (event.number == 1)
                    io.NavInputs[ImGuiNavInput_DpadUp] = 1.0f;
                }
              if (event.value > max_thresh[event.number]
                  && last_val[event.number] <= max_thresh[event.number])
                {
                  if (event.number == 0)
                    io.NavInputs[ImGuiNavInput_DpadRight] = 1.0f;
                  else if (event.number == 1)
                    io.NavInputs[ImGuiNavInput_DpadDown] = 1.0f;
                }
              last_val[event.number] = event.value;
            }
          break;

        case JS_EVENT_BUTTON:
          if (event.value == 0) /* On button release */
            {
              switch (event.number)
                {
                case 0:
                  io.NavInputs[ImGuiNavInput_Activate] = 1.0f;
                  break;
                case 1:
                  io.NavInputs[ImGuiNavInput_Cancel] = 1.0f;
                  break;
                case 2:
                  io.NavInputs[ImGuiNavInput_KeyMenu_] = 1.0f;
                  break;
                }
            }
          break;
        }
    }
  // lseek(jm_joystick->fd, 0, SEEK_END);
}
