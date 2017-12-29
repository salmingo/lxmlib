bin_PROGRAMS=lxmlib
lxmlib_SOURCES= AMath.cpp ATimeSpace.cpp ADIProcess.cpp daemon.cpp GLog.cpp IOServiceKeep.cpp \
                MessageQueue.cpp NTPClient.cpp tcpasio.cpp udpasio.cpp SerialComm.cpp \
                WCSTNX.cpp \
                lxmlib.cpp

lxmlib_LDFLAGS = -L/usr/local/lib
BOOST_LIBS = -lboost_system-mt -lboost_thread-mt -lboost_filesystem-mt
lxmlib_LDADD = ${BOOST_LIBS} -lm -lpthread -lcfitsio 
