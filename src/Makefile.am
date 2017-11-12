bin_PROGRAMS=lxmlib
lxmlib_SOURCES= AMath.cpp ATimeSpace.cpp daemon.cpp GLog.cpp IOServiceKepp.cpp \
                MessageQueue.cpp NTPClient.cpp tcpasio.cpp \
                lxmlib.cpp

lxmlib_LDFLAGS = -L/usr/local/lib
BOOST_LIBS = -lboost_system-mt -lboost_thread-mt
lxmlib_LDADD = ${BOOST_LIBS} -lm -lpthread
