cmake_minimum_required(VERSION 3.6.2)

set(IMEGL_CMAKE "${CMAKE_SOURCE_DIR}/../src/include.cmake")

include(${IMEGL_CMAKE})

set(CMAKE_CXX_FLAGS "-Wall -Werror -g")
# set(CMAKE_CXX_FLAGS "-Wall -Werror -O3")

set(APP_NAME rpi_menu_dialog)

add_executable(${APP_NAME}
  rpi_menu_dialog.cpp
  ${IMEGL_SRC})

message(STATUS "IMEGL_LIBS: ${IMEGL_LIBRARIES}")

target_link_libraries(${APP_NAME}
  ${IMEGL_LIBRARIES})

target_include_directories(${APP_NAME}
  PUBLIC
  ${IMEGL_INC})
