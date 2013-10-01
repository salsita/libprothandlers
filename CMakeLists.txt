cmake_minimum_required (VERSION 2.8)
project (libprothandlers)

set(CMAKE_CONFIGURATION_TYPES Debug Release )
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Reset the configurations to those we need" FORCE)

#Variables describing project layout
set(${PROJECT_NAME}_INC_DIR "${${PROJECT_NAME}_SOURCE_DIR}/include" CACHE PATH "Path to ${PROJECT_NAME} include directory")
set(${PROJECT_NAME}_SRC_DIR_PROT_HANDLER "${${PROJECT_NAME}_SOURCE_DIR}/libprothandlers")
set(${PROJECT_NAME}_SRC_DIR_PROT_PATCH "${${PROJECT_NAME}_SOURCE_DIR}/ProtocolPatch")

include_directories(
  BEFORE
  ${${PROJECT_NAME}_SOURCE_DIR}
  ${${PROJECT_NAME}_INC_DIR}
  ${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}
  ${${PROJECT_NAME}_SRC_DIR_PROT_PATCH}
)

set(${PROJECT_NAME}_HEADERS
  "${${PROJECT_NAME}_INC_DIR}/ProtocolHandlerRegistrar.h"
  "${${PROJECT_NAME}_SOURCE_DIR}/stdafx.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/Buffer.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolFolderHandler.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolFolderHandlerClassFactory.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolHandlerClassFactoryT.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolHandlerT.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolResourceHandler.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolResourceHandlerClassFactory.h"

  "${${PROJECT_NAME}_INC_DIR}/ProtocolPatch.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCH}/GlobalMap.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCH}/IInternetProtocolExPatch.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCH}/Protocol.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCH}/Scheme.h"
  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCH}/URLMonPatchBase.h"
  )

set(${PROJECT_NAME}_SOURCES
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/ProtocolHandlerRegistrar.cpp"
  "${${PROJECT_NAME}_SOURCE_DIR}/stdafx.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/Buffer.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolFolderHandler.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolFolderHandlerClassFactory.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolResourceHandler.cpp"
  "${${PROJECT_NAME}_SRC_DIR_PROT_HANDLER}/TemporaryProtocolResourceHandlerClassFactory.cpp"

  "${${PROJECT_NAME}_SRC_DIR_PROT_PATCH}/ProtocolPatch.cpp"
  )

################# MAIN TARGET SETUP #########################

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES})

# set UNICODE
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS _UNICODE,UNICODE)

# _WIN64
if (${CMAKE_CL_64})
  add_definitions(-DWIN64 -D_WIN64)
endif (${CMAKE_CL_64})

# exclude default libraries
set_target_properties(${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS /NODEFAULTLIB)

set(${PROJECT_NAME}_LIB_DIR "${${PROJECT_NAME}_BINARY_DIR}"  CACHE PATH "Path to ${PROJECT_NAME} lib directory")

message(STATUS "Configuring ${PROJECT_NAME}")
