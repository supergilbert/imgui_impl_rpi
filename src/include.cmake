cmake_minimum_required(VERSION 3.6.2)

set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/../imgui)

set(IMGUI_SRC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/imgui_draw.cpp)

set(IMEGL_DIR ${CMAKE_CURRENT_LIST_DIR})

set(IMEGL_SRC
  ${IMGUI_SRC}
  ${IMEGL_DIR}/imegl.cpp
  ${IMEGL_DIR}/imgui_impl_egl.cpp
  ${IMEGL_DIR}/imgui_impl_gles2.cpp
  ${IMEGL_DIR}/imgui_impl_linux_joystick.cpp)

set(IMEGL_INC ${IMEGL_DIR} ${IMGUI_DIR})

find_package(PkgConfig)


if("${IMEGL_X11}" STREQUAL "YES")
  pkg_check_modules(X11   REQUIRED x11)
  pkg_check_modules(EGL   REQUIRED egl)
  pkg_check_modules(GLES2 REQUIRED glesv2)

  add_definitions(-DX11)

  set(IMEGL_LIBS
    ${X11_LIBRARIES}
    ${EGL_LIBRARIES}
    ${GLES2_LIBRARIES}
    ${IMEGL_APP_LIBS})
else()                          # BRCM part
  set(IMEGL_INC
    ${IMEGL_INC}
    "/opt/vc/include")
  
  find_library(BCM_HOST_LIB
    NAMES bcm_host
    PATHS "/opt/vc/lib")
  if(${BCM_HOST_LIB} STREQUAL "BCM_HOST_LIB-NOTFOUND")
    message(FATAL_ERROR "Unable to find bcm_host library.")
  endif()

  find_library(EGL_LIB
    NAMES EGL
    PATHS "/opt/vc/lib")
  if(${EGL_LIB} STREQUAL "EGL_LIB-NOTFOUND")
    message(FATAL_ERROR "Unable to find EGL library.")
  endif()

  find_library(GLES2_LIB
    NAMES GLESv2
    PATHS "/opt/vc/lib")
  if(${GLES2_LIB} STREQUAL "GLES2_LIB-NOTFOUND")
    message(FATAL_ERROR "Unable to find GLESv2 library.")
  endif()

  set(IMEGL_LIBS
    ${BCM_HOST_LIB}
    ${EGL_LIB}
    ${GLES2_LIB}
    ${APP_LIBS})
endif()
