#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "imegl.h"
#include "imegl_tools.h"
#include "imgui_impl_egl.h"

struct rpi_menu_item_t
{
  char *tag;
  char *label;
};

bool rpi_menu(rpi_menu_item_t *rpi_items,
              size_t items_count)
{
  bool bool_var = true;
  bool continue_loop = true;
  size_t idx;

#ifdef X11
  imegl_init("rpi menu", 640, 480);
#else
  imegl_init();
#endif
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = NULL;

  if (iie_joystick_available() == true)
    if (iie_joystick_calibrated() == false)
      iie_calibrate_joystick();

#ifdef X11
  while (imegl_x11_closed() == false && continue_loop)
#else
    while (continue_loop)
#endif
      {
        usleep(50000);
        imegl_new_frame();

        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::Begin("rpi_menu",
                     &bool_var,
                     ImGuiWindowFlags_NoTitleBar
                     | ImGuiWindowFlags_NoDecoration
                     | ImGuiWindowFlags_NoMove);

        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::SetNextWindowFocus();
        if (ImGui::ListBoxHeader("", ImVec2(-1, -1)))
          {
            ImGui::SetNextWindowBgAlpha(1.0f);
            for (idx = 0; idx < items_count; idx++)
              if (ImGui::Selectable(rpi_items[idx].label) == true)
                {
                  fprintf(stderr, "%s\n", rpi_items[idx].tag);
                  continue_loop = false;
                }
            ImGui::ListBoxFooter();
          }

        ImGui::End();
        ImGui::Render();
        imegl_render_draw_data(ImGui::GetDrawData());
        imegl_swap_buffer();
      }

  ImGui::DestroyContext();
  imegl_shutdown();

  return continue_loop == false;
}

rpi_menu_item_t *rpi_items_init(const char **labels_n_tags,
                                size_t count)
{
  size_t          idx = 0;
  rpi_menu_item_t *items =
    (rpi_menu_item_t *) malloc(count * sizeof (rpi_menu_item_t));

  while (idx < count)
    {
      items[idx].tag = strdup(*labels_n_tags);
      labels_n_tags++;
      items[idx].label = strdup(*labels_n_tags);
      labels_n_tags++;
      idx++;
    }

  return items;
}

void rpi_items_destroy(rpi_menu_item_t *items, size_t count)
{
  size_t idx;

  for (idx = 0;
       idx < count;
       idx++)
    {
      free((void *) items[idx].label);
      free((void *) items[idx].tag);
    }
  free(items);
}

int main(int argc, char **argv)
{
  rpi_menu_item_t *items = NULL;
  FILE *devnull_stream = fopen("/dev/null", "w");
  int idx;

  for (idx = 1; idx < argc; idx++)
    if (strcmp(argv[idx], "-h") == 0
        || strcmp(argv[idx], "--help") == 0)
      {
        log_msg("Usage: %s <tag1> <label1> <tag2> <label2> ...\n"
                "  Output the result in stderr (like dialog).",
                argv[0]);
        return 0;
      }

  msg_output_stream = devnull_stream;
  msg_output_stream = devnull_stream;

  argc -= 1;

  if (argc % 2 != 0)
    return 2;

  if (argc <= 1)
    return 3;

  argc = argc >> 1;

  items = rpi_items_init((const char **) &(argv[1]), argc);

  if (rpi_menu(items, argc))
    return 0;
  return 1;
}
