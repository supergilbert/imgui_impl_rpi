#ifndef IMGUI_IMPL_LINUX_JOYSTICK_TYPE_H
#define IMGUI_IMPL_LINUX_JOYSTICK_TYPE_H

#define LJ_PATH_MAX_LEN 256
#define LJ_NAME_MAX_LEN 64

class LinuxJoystickNav
{
private:
  bool calibrated, available;
  int joystick_fd;
  char joystick_name[LJ_PATH_MAX_LEN];
  float min_thresh[2];
  float max_thresh[2];
  bool LoadCalibrationFromPath(const char *calibration_path);
  bool LoadCalibration(void);
  bool SaveCalibrationToPath(const char *calibration_path);
  bool SaveCalibration(void);
public:
  LinuxJoystickNav(const char *dev_path);
  LinuxJoystickNav(void);
  ~LinuxJoystickNav(void);
  bool Calibrate(void);
  bool IsCalibrated(void);
  bool IsAvailable(void);
  bool Load(const char *calibration_path);
  void UpdateNav(void);
};

#endif  // IMGUI_IMPL_LINUX_JOYSTICK_TYPE_H
