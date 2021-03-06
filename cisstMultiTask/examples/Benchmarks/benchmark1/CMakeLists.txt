#
#
# (C) Copyright 2005-2011 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
# 
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
# 
# --- end cisst license ---

# name of project
project(mtsExBenchmark1)

set(REQUIRED_CISST_LIBRARIES cisstCommon cisstOSAbstraction cisstMultiTask cisstParameterTypes) 

# find cisst and make sure the required libraries have been compiled
find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # name the main executable and specifies with source files to use
  add_executable(mtsExBenchmark1
                 clientTask.cpp
                 serverTask.cpp
                 localMain.cpp
                 clientTask.h
                 serverTask.h
                 configuration.h
                 )
  set_property (TARGET mtsExBenchmark1 PROPERTY FOLDER "cisstMultiTask/examples")

  # link with the cisst libraries
  cisst_target_link_libraries(mtsExBenchmark1 ${REQUIRED_CISST_LIBRARIES})

  # compile network version if ICE is available
  if (CISST_MTS_HAS_ICE)

    # name the main executable and specifies with source files to use
    add_executable(mtsExBenchmark1Server
                   serverTask.cpp
                   serverMain.cpp
                   serverTask.h
                   configuration.h
                   )
    set_property (TARGET mtsExBenchmark1Server PROPERTY FOLDER "cisstMultiTask/examples")

    # link with the cisst libraries
    cisst_target_link_libraries(mtsExBenchmark1Server ${REQUIRED_CISST_LIBRARIES})

    # name the main executable and specifies with source files to use
    add_executable(mtsExBenchmark1Client
                   clientTask.cpp
                   clientMain.cpp
                   clientTask.h
                   configuration.h
                   )
    set_property (TARGET mtsExBenchmark1Client PROPERTY FOLDER "cisstMultiTask/examples")

    # link with the cisst libraries
    cisst_target_link_libraries(mtsExBenchmark1Client ${REQUIRED_CISST_LIBRARIES})

  endif (CISST_MTS_HAS_ICE)

else (cisst_FOUND_AS_REQUIRED)
  message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND_AS_REQUIRED)
