cmake_minimum_required(VERSION 3.6.2)

if("${IMEGL_CMAKE}" STREQUAL "")
  message(FATAL_ERROR "No variable IMEGL_CMAKE defined.")
endif()

if(NOT EXISTS ${IMEGL_CMAKE})
  message("Unable to find ${IMEGL_CMAKE}")
endif()

include(${IMEGL_CMAKE})

# set(CMAKE_CXX_FLAGS "-Wall -Werror -g")
# set(CMAKE_CXX_FLAGS "-Wall -Werror -O3")

set(APP_NAME rpi_menu_dialog)

add_executable(${APP_NAME}
  rpi_menu_dialog.cpp
  ${IMEGL_SRC})

target_link_libraries(${APP_NAME}
  ${IMEGL_LIBS})

target_include_directories(${APP_NAME}
  PUBLIC
  ${IMEGL_INC})
