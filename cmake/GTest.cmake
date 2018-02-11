find_package ( Threads REQUIRED )
include ( ExternalProject )
set ( GTEST_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/external/gtest/install" )

set ( LIBPREFIX "${CMAKE_STATIC_LIBRARY_PREFIX}" )
set ( LIBSUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}" )
set ( GTEST_LOCATION "${GTEST_PREFIX}/lib/" )
set ( GTEST_INCLUDES "${GTEST_PREFIX}/include" )
set ( GTEST_LIBRARY "${GTEST_LOCATION}/${LIBPREFIX}gtest${LIBSUFFIX}" )

set ( GMOCK_INCLUDES "${GTEST_PREFIX}/include" )
set ( GMOCK_LIBRARY "${GTEST_LOCATION}/${LIBPREFIX}gmock${LIBSUFFIX}" )

externalproject_add ( GTestExternal
                      URL https://github.com/google/googletest/archive/release-1.8.0.zip
                      URL_HASH SHA1=667f873ab7a4d246062565fad32fb6d8e203ee73
                      TIMEOUT 600e
                      PREFIX "external/gtest"
                      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/external/
                      EXCLUDE_FROM_ALL 1 CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${GTEST_PREFIX}
                      BUILD_BYPRODUCTS ${GTEST_LIBRARY} ${GMOCK_LIBRARY} )

add_library ( GTest INTERFACE )
target_include_directories ( GTest SYSTEM INTERFACE "${GTEST_INCLUDES}" )
target_link_libraries ( GTest INTERFACE "${GTEST_LIBRARY}" Threads::Threads )

add_library ( GMock INTERFACE )
target_include_directories ( GMock SYSTEM INTERFACE "${GMOCK_INCLUDES}" )
target_link_libraries ( GMock INTERFACE "${GMOCK_LIBRARY}" Threads::Threads )

add_dependencies ( GTest GTestExternal )
add_dependencies ( GMock GTestExternal )
