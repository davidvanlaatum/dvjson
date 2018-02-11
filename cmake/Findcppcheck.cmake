find_program ( CPPCHECK_PATH cppcheck )
set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )
option ( CPPCHECK "Enable cppcheck" ON )
option ( CPPCHECK_XML "Enable cppcheck xml" OFF )
if ( CPPCHECK AND CPPCHECK_PATH )
  set ( CPPCHECK_ARGS --suppress=*:${CMAKE_SOURCE_DIR}/src/json/json.hpp --platform=unix64 --inline-suppr
        --project=${CMAKE_BINARY_DIR}/cppcheck_compile_commands.json --max-configs=10 -I/usr/include --config-exclude=/usr/include
        -rp --std=c++11 --enable=performance,portability --config-excludes-file=${CMAKE_BINARY_DIR}/config_exclude_cppcheck -UENUMTEST -UMOCK_METHOD0 -UFRIEND_TEST
        --config-exclude=${CMAKE_SOURCE_DIR}/src/json/ )

  set_source_files_properties ( ${CMAKE_BINARY_DIR}/compile_commands.json PROPERTIES GENERATED TRUE )
  add_custom_command ( OUTPUT ${CMAKE_BINARY_DIR}/config_exclude_cppcheck
                       COMMAND ${CMAKE_SOURCE_DIR}/scripts/extract_includes.pl compile_commands.json ${CMAKE_SOURCE_DIR}/src > config_exclude_cppcheck
                       MAIN_DEPENDENCY ${CMAKE_BINARY_DIR}/compile_commands.json
                       DEPENDS ${CMAKE_SOURCE_DIR}/scripts/extract_includes.pl ${CMAKE_CURRENT_LIST_FILE} )
  add_custom_command ( OUTPUT ${CMAKE_BINARY_DIR}/cppcheck_compile_commands.json
                       COMMAND sed -e "'s/-isystem \\([^ ]*\\)/-I\\1/g'" -e 's/-D[a-zA-Z=_\\\"\\/]* *//g' ${CMAKE_BINARY_DIR}/compile_commands.json > ${CMAKE_BINARY_DIR}/cppcheck_compile_commands.json
                       MAIN_DEPENDENCY ${CMAKE_BINARY_DIR}/compile_commands.json DEPENDS ${CMAKE_CURRENT_LIST_FILE} )

  if ( CPPCHECK_XML )
    set ( CPPCHECK_ARGS --xml --xml-version=2 ${CPPCHECK_ARGS} 2> cppcheckout.xml )
  endif ()
  add_custom_target ( cppcheck
                      COMMAND ${CPPCHECK_PATH} ${CPPCHECK_ARGS}
                      DEPENDS ${CMAKE_BINARY_DIR}/cppcheck_compile_commands.json ${CMAKE_BINARY_DIR}/config_exclude_cppcheck
                      )
  add_dependencies ( checks cppcheck )
endif ()
if ( CPPCHECK_PATH AND NOT CPPCHECK_VERSION )
  execute_process ( COMMAND ${CPPCHECK_PATH} --version
                    OUTPUT_VARIABLE CPPCHECK_VERSION
                    RESULT_VARIABLE EXIT_CODE
                    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET )
  string ( REGEX MATCH "[0-9\\.]+$" CPPCHECK_VERSION ${CPPCHECK_VERSION} )
  set ( CPPCHECK_VERSION ${CPPCHECK_VERSION} CACHE STRING "cppcheck version" )
endif ()

find_package_handle_standard_args ( Cppcheck
                                    FOUND_VAR Cppcheck_FOUND
                                    REQUIRED_VARS CPPCHECK_PATH CPPCHECK
                                    VERSION_VAR CPPCHECK_VERSION
                                    )
