# Find the JsonCpp include files and library.
#
# JsonCpp is a C++ library that can read/write JSON (JavaScript Object Notation)
# documents. See http://jsoncpp.sourceforge.net/ for more details.
#
# This module defines:
# JsonCpp_INCLUDE_DIRS - where to find json/json.h
# JsonCpp_LIBRARIES - the libraries to link against to use JsonCpp
# JsonCpp_FOUND - if false the library was not found.

find_path(JsonCpp_INCLUDE_DIR "json/json.h"
  PATHS ~/.linuxbrew/include
  PATH_SUFFIXES "jsoncpp"
  DOC "Specify the JsonCpp include directory here")

if(SBOL_BUILD_SHARED)
if(JsonCpp_LIBRARY MATCHES "(.*).(a|lib)")
unset(JsonCpp_LIBRARY CACHE)
endif()
else()
if(JsonCpp_LIBRARY MATCHES "(.*).(so|dylib|dll)")
unset(JsonCpp_LIBRARY CACHE)
endif()
endif()

find_library(JsonCpp_LIBRARY
  NAMES jsoncpp
  PATHS ~/.linuxbrew/lib  # linuxbrew
  DOC "Specify the JsonCpp library here")
set(JsonCpp_INCLUDE_DIRS ${JsonCpp_INCLUDE_DIR})
set(JsonCpp_LIBRARIES "${JsonCpp_LIBRARY}")
message("JsonCpp_INCLUDE_DIR: ${JsonCpp_INCLUDE_DIR}")
set(_JsonCpp_version_args)
if (EXISTS "${JsonCpp_INCLUDE_DIR}/json/version.h")
  file(STRINGS "${JsonCpp_INCLUDE_DIR}/json/version.h" _JsonCpp_version_contents REGEX "JSONCPP_VERSION_[A-Z]+")
  foreach (_JsonCpp_version_part MAJOR MINOR PATCH)
    string(REGEX REPLACE ".*# *define +JSONCPP_VERSION_${_JsonCpp_version_part} +([0-9]+).*" "\\1" JsonCpp_VERSION_${_JsonCpp_version_part} "${_JsonCpp_version_contents}")
  endforeach ()

  set(JsonCpp_VERSION_STRING "${JsonCpp_VERSION_MAJOR}.${JsonCpp_VERSION_MINOR}.${JsonCpp_VERSION_PATCH}")

  set(_JsonCpp_version_args VERSION_VAR JsonCpp_VERSION_STRING)
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JsonCpp
  REQUIRED_VARS JsonCpp_LIBRARIES JsonCpp_INCLUDE_DIRS
  ${_JsonCpp_version_args})

mark_as_advanced(JsonCpp_INCLUDE_DIR JsonCpp_LIBRARY)
