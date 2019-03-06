#!/bin/sh

NAME=$(basename $0)
CURRENT_DIR=$(realpath $(dirname $0))
BUILD_DIR=${CURRENT_DIR}/build

synopsis ()
{
    echo "Usage: $NAME [-h|--help] [-x|--x11|--X11]"
    echo "  Build imegl binary."
    echo "  -h --help      Display this help section and exit."
    echo "  -x --x11 --X11 Set the target platform to X11 (not the default"
    echo "                 BRCM RPI)."
}

OPTS=$(getopt -n $NAME -o 'hx' -l 'help,x11,X11' -- "$@")
#Bad arguments
if [ $? -ne 0 ]; then
    synopsis >&2
    echo "Bad arguments." >&2
    exit 1
fi
eval set -- "$OPTS";
while true; do
    case "$1" in
        "-h"|"--help")
            shift
            synopsis
            exit 0
            ;;
        "-x"|"--x11"|"--X11")
            shift
            CMAKE_ARGS="-DIMEGL_X11=YES"
            ;;
        --)
            shift
            break
            ;;
    esac
done

git submodule init
git submodule update

test -d "$BUILD_DIR" || mkdir "$BUILD_DIR"
cd "$BUILD_DIR"
if cmake $CMAKE_ARGS -DIMEGL_CMAKE=../src/include.cmake ../rpi_menu; then
    cmake --build .
fi
