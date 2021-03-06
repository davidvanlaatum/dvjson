if ( SUBBUILD_DVJSON )
  message ( STATUS "Begin dvjson" )
endif ()

list ( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" )
cmake_minimum_required ( VERSION 3.6 )

foreach ( MAJOR RANGE 10 3 -1 )
  foreach ( MINOR RANGE 10 0 -1 )
    list ( APPEND CLANG_ALIASES clang-${MAJOR}.${MINOR} )
    list ( APPEND CLANGPP_ALIASES clang++-${MAJOR}.${MINOR} )
  endforeach ()
endforeach ()

find_program ( CLANG NAMES clang ${CLANG_ALIASES} HINTS /opt/local/bin )
find_program ( CLANGPP NAMES clang++ ${CLANGPP_ALIASES} HINTS /opt/local/bin )
set ( CXX $ENV{CXX} )
set ( CC $ENV{CC} )
if ( CLANG AND CLANGPP AND ( NOT CC ) AND ( NOT CXX ) AND NOT CMAKE_CXX_COMPILER AND NOT CMAKE_C_COMPILER )
  if ( NOT CMAKE_C_COMPILER )
    set ( CMAKE_C_COMPILER ${CLANG} )
  endif ()
  if ( NOT CMAKE_CXX_COMPILER )
    set ( CMAKE_CXX_COMPILER ${CLANGPP} )
  endif ()
  message ( STATUS "Found clang" )
endif ()

project ( dvjson VERSION 1.0.0 )
if ( DEFINED SUBBUILD_DVJSON )
  set ( dvjson_VERSION "${dvjson_VERSION}" PARENT_SCOPE )
endif ()
set ( CMAKE_CXX_STANDARD 11 )
set ( CMAKE_CXX_STANDARD_REQUIRED ON )
if ( NOT ${CMAKE_VERSION} VERSION_LESS 3.9 )
  cmake_policy ( SET CMP0068 NEW )
endif ()

if ( NOT DEFINED SUBBUILD_DVJSON )
  add_custom_target ( checks )
endif ()

include ( FeatureSummary )
include ( GNUInstallDirs )
include ( CheckCXXCompilerFlag )
include ( GenerateExportHeader )
if ( NOT DEFINED SUBBUILD_DVJSON )
  include ( CTest )
  include ( GTest )
endif ()
include ( CheckAndAddFlag )
if ( NOT DEFINED SUBBUILD_DVJSON )
  find_package ( valgrind )
  find_package ( iwyu )
  find_package ( cppcheck )
  find_package ( clang-tidy )
endif ()
find_package ( Boost 1.60 REQUIRED )
if ( NOT TARGET Boost::boost )
  message ( FATAL_ERROR "Boost cmake module not found!" )
endif ()

checkandaddflag ( -Weverything CMAKE_CXX_FLAGS )
checkandaddflag ( -Wall CMAKE_CXX_FLAGS )
checkandaddflag ( -Wextra CMAKE_CXX_FLAGS )
checkandaddflag ( -fdebug-macro CMAKE_CXX_FLAGS )
checkandaddflag ( -fmerge-constants CMAKE_CXX_FLAGS )
checkandaddflag ( -fspell-checking CMAKE_CXX_FLAGS )
checkandaddflag ( -Wall CMAKE_CXX_FLAGS )
checkandaddflag ( -Wextra CMAKE_CXX_FLAGS )
checkandaddflag ( -Wunused CMAKE_CXX_FLAGS )
checkandaddflag ( -Wuninitialized CMAKE_CXX_FLAGS )
checkandaddflag ( -Wmaybe-uninitialized CMAKE_CXX_FLAGS )
checkandaddflag ( -Winit-self CMAKE_CXX_FLAGS )
checkandaddflag ( -Wfloat-equal CMAKE_CXX_FLAGS )
checkandaddflag ( -Wshadow CMAKE_CXX_FLAGS )
checkandaddflag ( -Wshadow-all CMAKE_CXX_FLAGS )
checkandaddflag ( -Wsign-conversion CMAKE_CXX_FLAGS )
checkandaddflag ( -Wpedantic CMAKE_CXX_FLAGS )
checkandaddflag ( -Wno-variadic-macros CMAKE_CXX_FLAGS )
checkandaddflag ( -Wno-long-long CMAKE_CXX_FLAGS )
checkandaddflag ( -Wno-c99-extensions CMAKE_CXX_FLAGS )
checkandaddflag ( -Wno-unused-command-line-argument CMAKE_CXX_FLAGS )
checkandaddflag ( -fdiagnostics-show-template-tree CMAKE_CXX_FLAGS )
checkandaddflag ( -Qunused-arguments CMAKE_CXX_FLAGS )
checkandaddflag ( -Wno-weak-vtables CMAKE_CXX_FLAGS )
checkandaddflag ( -Wno-c++98-compat CMAKE_CXX_FLAGS )
checkandaddflag ( -Wno-global-constructors CMAKE_CXX_FLAGS )
checkandaddflag ( -Wno-global-constructors CMAKE_CXX_FLAGS )
checkandaddflag ( -Wno-zero-as-null-pointer-constant CMAKE_CXX_FLAGS )
checkandaddflag ( -Wno-unused-command-line-argument CMAKE_CXX_FLAGS )
checkandaddflag ( -Wno-padded CMAKE_CXX_FLAGS )
checkandaddflag ( -Wno-c++98-compat-pedantic CMAKE_CXX_FLAGS )
checkandaddflag ( -Wno-exit-time-destructors CMAKE_CXX_FLAGS )
checkandaddflag ( -Wfatal-errors CMAKE_CXX_FLAGS )
checkandaddflag ( -fmessage-length=160 CMAKE_CXX_FLAGS )
checkandaddflag ( -Wsuggest-override CMAKE_CXX_FLAGS )
checkandaddflag ( -Wduplicated-cond CMAKE_CXX_FLAGS )
checkandaddflag ( -Wduplicated-branches CMAKE_CXX_FLAGS )
checkandaddflag ( -fdiagnostics-color CMAKE_CXX_FLAGS )
checkandaddflag ( -fcolor-diagnostics CMAKE_CXX_FLAGS )

add_subdirectory ( src )
if (( NOT DEFINED SUBBUILD_DVJSON OR SUBBUILD_TESTING ) AND BUILD_TESTING )
  add_subdirectory ( test )
endif ()

include ( CMakePackageConfigHelpers )
write_basic_package_version_file ( "${CMAKE_CURRENT_BINARY_DIR}/dvjsonVersion.cmake"
                                   VERSION ${PROJECT_VERSION} COMPATIBILITY AnyNewerVersion )
export ( EXPORT dvjson FILE "${CMAKE_CURRENT_BINARY_DIR}/dvjsonConfig.cmake" )
install ( EXPORT dvjson
          FILE dvjsonConfig.cmake
          COMPONENT Devel
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dvjson
          )
install ( FILES "${CMAKE_CURRENT_BINARY_DIR}/dvjsonVersion.cmake"
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dvjson
          COMPONENT Devel
          )
if ( NOT DEFINED SUBBUILD_DVJSON )
  feature_summary ( WHAT ALL )
  message ( STATUS "CXX=${CMAKE_CXX_COMPILER} CC=${CMAKE_C_COMPILER}" )
endif ()
export ( PACKAGE dvjson )
if ( NOT DEFINED SUBBUILD_DVJSON )
  set ( CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} )
  set ( CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR} )
  set ( CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH} )
  set ( CPACK_PACKAGE_VENDOR "David van Laatum" )
  set ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "dvjson C++ library" )
  set ( CPACK_PROJECT_CONFIG_FILE ${CMAKE_SOURCE_DIR}/CPack.cmake )
  include ( CPack )
  include ( CPackComponent )

  foreach ( C ${CPACK_COMPONENTS_ALL} )
    list ( APPEND COMMANDS COMMAND ${CMAKE_COMMAND} -E env DESTDIR=installtest ${CMAKE_COMMAND} -DCOMPONENT=${C} -P ${CMAKE_BINARY_DIR}/cmake_install.cmake )
  endforeach ()
  if ( Unspecified IN_LIST CPACK_COMPONENTS_ALL )
    message ( WARNING "Files that have not been assigned to a category exist" )
  endif ()
  if ( BUILD_TESTING )
    add_custom_target ( installtest
                        COMMAND ${CMAKE_COMMAND} -E remove_directory installtest
                        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/install_manifest*
                        ${COMMANDS}
                        )
  endif ()
endif ()

if ( SUBBUILD_DVJSON )
  message ( STATUS "End dvjson" )
endif ()
