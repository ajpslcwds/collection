cmake_minimum_required(VERSION 3.10)
############FOR_MODIFIY_BEGIN#######################
PROJECT (drsdk_fake_ds)

INCLUDE($ENV{DRDIR}CMakeCommon)

#Setting Source Files
SET(SRCS ${SRCS} fake_ds.cpp)
#Setting Target Name (executable file name | library name)
SET(TARGET_NAME fake_ds)
#Setting library type used when build a library
#SET(LIB_TYPE SHARED)

# Á´½ÓDRSdk¿â
include_directories(${CMAKE_SOURCE_DIR}/DRSdk/drsdk)
SET(LINK_LIBS drsdk cv6hdProcComm)

IF(UNIX)
	SET(LINK_LIBS ${LINK_LIBS} rt boost_system boost_thread boost_chrono)
	IF(CMAKE_SYSTEM MATCHES "SunOS.*")
		SET(LINK_LIBS ${LINK_LIBS} socket nsl )
	ENDIF(CMAKE_SYSTEM MATCHES "SunOS.*")
	IF(HPUX)
		SET(LINK_LIBS ${LINK_LIBS} pthread ipv6)
	ENDIF(HPUX)
	if( CMAKE_SIZEOF_VOID_P EQUAL 4 )
		IF(CMAKE_SYSTEM MATCHES "Linux")
			SET(LINK_LIBS ${LINK_LIBS} SentinelKeys32)
		ENDIF(CMAKE_SYSTEM MATCHES "Linux")
	endif( CMAKE_SIZEOF_VOID_P EQUAL 4 )
	IF(CMAKE_SYSTEM MATCHES "Linux")
			SET(LINK_LIBS ${LINK_LIBS} pthread)
	ENDIF(CMAKE_SYSTEM MATCHES "Linux")
ENDIF(UNIX)

############FOR_MODIFIY_END#########################
INCLUDE($ENV{DRDIR}CMakeCommonExec)

