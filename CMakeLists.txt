cmake_minimum_required (VERSION 2.8)
project (libprothandlers)

set(${PROJECT_NAME}_INC_DIRS "${${PROJECT_NAME}_SOURCE_DIR}/include")
set(${PROJECT_NAME}_SRC_DIR "${${PROJECT_NAME}_SOURCE_DIR}/libprothandlers")

set(_${PROJECT_NAME}_INTERNAL_INC_DIR ${${PROJECT_NAME}_SRC_DIR})

include_directories("${${PROJECT_NAME}_INC_DIRS}")

set(${PROJECT_NAME}_HEADERS
	"${${PROJECT_NAME}_INC_DIRS}/ProtocolHandlerRegistrar.h"
	"${_${PROJECT_NAME}_INTERNAL_INC_DIR}/stdafx.h"
	"${_${PROJECT_NAME}_INTERNAL_INC_DIR}/TemporaryProtocolFolderHandler.h"
	"${_${PROJECT_NAME}_INTERNAL_INC_DIR}/TemporaryProtocolFolderHandlerClassFactory.h"
	"${_${PROJECT_NAME}_INTERNAL_INC_DIR}/TemporaryProtocolHandlerClassFactoryT.h"
	"${_${PROJECT_NAME}_INTERNAL_INC_DIR}/TemporaryProtocolHandlerT.h"
	"${_${PROJECT_NAME}_INTERNAL_INC_DIR}/TemporaryProtocolResourceHandler.h"
	"${_${PROJECT_NAME}_INTERNAL_INC_DIR}/TemporaryProtocolResourceHandlerClassFactory.h"
	)

set(${PROJECT_NAME}_SOURCES 
	"${${PROJECT_NAME}_SRC_DIR}/ProtocolHandlerRegistrar.cpp"
	"${${PROJECT_NAME}_SRC_DIR}/stdafx.cpp"
	"${${PROJECT_NAME}_SRC_DIR}/TemporaryProtocolFolderHandler.cpp"
	"${${PROJECT_NAME}_SRC_DIR}/TemporaryProtocolFolderHandlerClassFactory.cpp"
	"${${PROJECT_NAME}_SRC_DIR}/TemporaryProtocolResourceHandler.cpp"
	"${${PROJECT_NAME}_SRC_DIR}/TemporaryProtocolResourceHandlerClassFactory.cpp"
	)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES})

# set UNICODE
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS _UNICODE,UNICODE)

# exclude default libraries
set_target_properties(${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS /NODEFAULTLIB)

set(${PROJECT_NAME}_INC_DIRS ${${PROJECT_NAME}_INC_DIRS} PARENT_SCOPE)

message(STATUS "Generated ${PROJECT_NAME}")
