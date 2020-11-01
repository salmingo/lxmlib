bin_PROGRAMS=lxmlib
lxmlib_SOURCES=GLog.cpp AsioIOServiceKeep.cpp MessageQueue.cpp CurlBase.cpp AsioTCP.cpp AsioUDP.cpp \
               BuildMatchingShape.cpp lxmlib.cpp

# lxmlib_CPPFLAGS=-I../../include
# lxmlib_LDFLAGS=-L../../lib
# lxmlib_LDADD=-lm -lsofa_c
if DEBUG
  AM_CFLAGS = -g3 -O0 -Wall -DNDEBUG
  AM_CXXFLAGS = -g3 -O0 -Wall -DNDEBUG
else
  AM_CFLAGS = -O3 -Wall
  AM_CXXFLAGS = -O3 -Wall
endif

if WINDOWS
  AM_CFLAGS += -DWINDOWS
  AM_CXXFLAGS += -DWINDOWS
endif

lxmlib_LDFLAGS = -L/usr/local/lib
BOOST_LIBS = -lboost_thread-mt
lxmlib_LDADD = ${BOOST_LIBS} -lcurl
if LINUX
lxmlib_LDADD += -lrt
endif
