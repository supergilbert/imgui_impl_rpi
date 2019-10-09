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

  set(IMEGL_LIBRARIES
    ${X11_LIBRARIES}
    ${EGL_LIBRARIES}
    ${GLES2_LIBRARIES})

else()                          # BRCM part
  set(IMEGL_INC
    ${IMEGL_INC}
    "/opt/vc/include")
  
  find_library(BCM_HOST_LIBRARIES
    NAMES bcm_host
    PATHS "/opt/vc/lib")
  if(${BCM_HOST_LIBRARIES} STREQUAL "BCM_HOST_LIB-NOTFOUND")
    message(FATAL_ERROR "Unable to find bcm_host library.")
  endif()

  # pkg_check_modules(EGL   REQUIRED egl)
  # pkg_check_modules(GLES2 REQUIRED glesv2)

  find_library(EGL_LIBRARIES
    NAMES brcmEGL
    PATHS "/opt/vc/lib")
  if(${EGL_LIBRARIES} STREQUAL "EGL_LIBRARIES-NOTFOUND")
    message(FATAL_ERROR "Unable to find EGL library.")
  endif()

  find_library(GLES2_LIBRARIES
    NAMES brcmGLESv2
    PATHS "/opt/vc/lib")
  if(${GLES2_LIBRARIES} STREQUAL "GLES2_LIBRARIES-NOTFOUND")
    message(FATAL_ERROR "Unable to find GLESv2 library.")
  endif()

  set(IMEGL_LIBRARIES
    ${BCM_HOST_LIBRARIES}
    ${EGL_LIBRARIES}
    ${GLES2_LIBRARIES})
endif()
