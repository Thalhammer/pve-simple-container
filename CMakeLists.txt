cmake_minimum_required(VERSION 3.10)
project(pve-simple-container)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions(-DPICOJSON_USE_INT64)

add_compile_options(-Wall)
add_compile_options(-Wno-unknown-pragmas)

# Enable Link-Time Optimization
if(NOT ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTOAvailable)
endif()
if(LTOAvailable)
    message("Link-time optimization enabled")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Detect version info
set(GIT_VERSION "v0.0.0-0-0")
set(GIT_BRANCH "master")
set(GCC_VERSION "unknown")
find_package(Git)
if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --dirty --always --tags
    WORKING_DIRECTORY "${local_dir}"
    OUTPUT_VARIABLE GIT_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY "${local_dir}"
    OUTPUT_VARIABLE GIT_BRANCH
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()
execute_process(
  COMMAND bash -c "${CMAKE_C_COMPILER} --version | head -n 1"
  WORKING_DIRECTORY "${local_dir}"
  OUTPUT_VARIABLE GCC_VERSION
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

#parse the version information into pieces.
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${GIT_VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${GIT_VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${GIT_VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-([0-9]+).*" "\\1" VERSION_COMMIT "${GIT_VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-[0-9]+-(.*)" "\\1" VERSION_SHA1 "${GIT_VERSION}")
set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
message(${VERSION_SHORT})

add_definitions(-DPICOJSON_USE_INT64)
add_definitions(-DGCC_VERSION="${GCC_VERSION}")
add_definitions(-DGIT_VERSION="${GIT_VERSION}")

configure_file(pvesc/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)

add_executable(pvesc
    common/filesystem.cpp
    common/system.cpp
    common/webclient.cpp
    container/app.cpp
    container/overlay.cpp
    container/recipe.cpp
    deploy/apiclient.cpp
    deploy/app.cpp
    deploy/config.cpp
    pvesc/app.cpp
)
target_link_libraries(pvesc stdc++fs pthread ssl crypto z dl curl)
target_include_directories(pvesc
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/
    PRIVARE ${CMAKE_CURRENT_BINARY_DIR}/
)

install(TARGETS pvesc DESTINATION bin)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/pvesc-completion.sh"
        DESTINATION share/bash-completion/completions)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/baseimage.tar.gz"
        DESTINATION share/pve-simple-container/)
file(GLOB OVERLAY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/overlays/*)
install(FILES ${OVERLAY_FILES}
        DESTINATION share/pve-simple-container/overlays)

# Debs
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_PACKAGE_NAME "pve-simple-container")
set(CPACK_PACKAGE_VENDOR "Dominik Thalhammer")
set(CPACK_PACKAGE_CONTACT "dominik@thalhammer.it")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A small utility to allow docker like deployment of single application containers to a unmodified pve host.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Thalhammer/pve-simple-container")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dominik Thalhammer <dominik@thalhammer.it>")
set(CPACK_DEBIAN_PACKAGE_VERSION "${VERSION_SHORT}")
set(CPACK_DEBIAN_PACKAGE_RELEASE "${VERSION_COMMIT}")
set(CPACK_SET_DESTDIR true)
set(CPACK_INSTALL_PREFIX "/usr")

execute_process(
    COMMAND dpkg --print-architecture
    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
    OUTPUT_STRIP_TRAILING_WHITESPACE)

if (UNIX)
    #set(CPACK_GENERATOR "DEB;STGZ;TGZ;TZ")
    set(CPACK_GENERATOR "DEB")
endif (UNIX)

include(CPack)