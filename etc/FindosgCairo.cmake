# This is part of the Findosg* suite used to find OpenSceneGraph components.
# Each component is separate and you must opt in to each module. You must
# also opt into OpenGL and OpenThreads (and Producer if needed) as these
# modules won't do it for you. This is to allow you control over your own
# system piece by piece in case you need to opt out of certain components
# or change the Find behavior for a particular module (perhaps because the
# default FindOpenGL.cmake module doesn't work with your system as an
# example).
# If you want to use a more convenient module that includes everything,
# use the FindOpenSceneGraph.cmake instead of the Findosg*.cmake modules.
#
# Locate osgOcean
# This module defines
# OSGCAIRO_LIBRARY
# OSGCAIRO_FOUND, if false, do not try to link to osgOcean
# OSGCAIRO_INCLUDE_DIR, where to find the headers
#
# $OSGDIR is an environment variable that would
# correspond to the ./configure --prefix=$OSGDIR
# used in building osg.
#
# Created by Eric Wing, modified by Robert Osfield for osgOcean

# Header files are presumed to be included like
# #include <osgOcean/EphemerisModel>

# Try the user's environment request before anything else.
FIND_PATH(OSGCAIRO_INCLUDE_DIR osgCairo/Image
  HINTS
  $ENV{OSGCAIRO_DIR}
  $ENV{OSGCAIRO_DIR}/include
  $ENV{OSG_DIR}
  $ENV{OSGDIR}
  PATH_SUFFIXES include
  PATHS
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OpenThreads_ROOT]
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]
)

FIND_LIBRARY(OSGCAIRO_LIBRARY
  NAMES osgCairo
  HINTS
  $ENV{OSGCAIRO_DIR}
  $ENV{OSGCAIRO_DIR}/lib
  $ENV{OSG_DIR}
  $ENV{OSGDIR}
  PATH_SUFFIXES lib64 lib
  PATHS
    ~/Library/Frameworks
    /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

SET(OSGCAIRO_FOUND "NO")
IF(OSGCAIRO_LIBRARY AND OSGCAIRO_INCLUDE_DIR)
  SET(OSGCAIRO_FOUND "YES")
ENDIF(OSGCAIRO_LIBRARY AND OSGCAIRO_INCLUDE_DIR)

