AUTOMAKE_OPTIONS = foreign
SUBDIRS = data debian

LDADD=${X_LIBS} ${OSSO_LIBS} -lEGL -lGLESv2
CFLAGS=${X_CFLAGS} ${X_CLAGS} -Wall -g -O3 -DSUPPORT_X11 -DPREFIX=\"$(prefix)\"
CXXFLAGS=${X_CFLAGS} ${OSSO_CFLAGS} -Wall -g -O3 -DSUPPORT_X11 -DPREFIX=\"$(prefix)\"

bin_PROGRAMS = glmemperf
bin_SCRIPTS = glmemperf-create-ref.sh
glmemperf_SOURCES = \
    runner_cli.cpp \
    native_x11.c \
    util.cpp \
    test.cpp \
    cleartest.cpp \
    blittest.cpp \
    pixmapblittest.cpp \
    blitmultitest.cpp \
    fboblittest.cpp \
    shaderblittest.cpp \
    cpuinterleavingtest.cpp

noinst_HEADERS = \
    blittest.h \
    cleartest.h \
    cpuinterleavingtest.h \
    fboblittest.h \
    native.h \
    pixmapblittest.h \
    shaderblittest.h \
    test.h \
    util.h
