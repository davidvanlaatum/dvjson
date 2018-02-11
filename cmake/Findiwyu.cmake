include ( copy_target_properties )

option ( IWYU "Enable include what you use" ON )
option ( DISABLE_COMPILE "Switch off normal compiling" OFF )

if ( IWYU )
  find_program ( IWYU_PATH NAMES iwyu include-what-you-use )
  if ( IWYU_PATH )
    execute_process ( COMMAND ${CMAKE_CXX_COMPILER} -E -x c++ - -v INPUT_FILE /dev/null ERROR_VARIABLE OUT OUTPUT_VARIABLE DISCARD )
    if ( NOT CXX_DEFAULT_INCLUDES )
      string ( REGEX REPLACE "\n" ";" OUT "${OUT}" )
      foreach ( L ${OUT} )
        if ( L MATCHES "^ (/[^ ]+)" )
          list ( APPEND INCLUDES "${CMAKE_MATCH_1}" )
        endif ()
      endforeach ()
      set ( CXX_DEFAULT_INCLUDES ${INCLUDES} CACHE STRING "Default include path for use by IWYU" )
    endif ()
    configure_file ( scripts/iwyu.sh iwyu.sh @ONLY )
    configure_file ( scripts/iwyu_combine.sh iwyu_combine.sh @ONLY )
    add_custom_target ( iwyu )
    add_dependencies ( checks iwyu )
  endif ()
endif ()

find_package_handle_standard_args ( iwyu
                                    FOUND_VAR iwyu_FOUND
                                    REQUIRED_VARS IWYU_PATH
                                    )
function ( add_for_headers TARGET )
  get_target_property ( SOURCES ${TARGET} SOURCES )
  foreach ( S ${SOURCES} )
    get_filename_component ( EXT ${S} EXT )
    if ( EXT STREQUAL ".h" )
      get_source_file_property ( LANGUAGE ${S} LANGUAGE )
      get_filename_component ( BASE ${S} NAME_WE )
      get_filename_component ( DIRECTORY ${S} DIRECTORY )
      if ( DIRECTORY )
        set ( DIRECTORY "${DIRECTORY}/" )
      endif ()
      if ( NOT LANGUAGE AND NOT ${DIRECTORY}${BASE}.cpp IN_LIST SOURCES )
        get_filename_component ( PATH ${S} ABSOLUTE )
        file ( WRITE "${CMAKE_CURRENT_BINARY_DIR}/${BASE}.cpp" "#include \"${PATH}\"\n" )
        target_sources ( ${TARGET} PRIVATE ${BASE}.cpp )
      endif ()
    endif ()
  endforeach ()
endfunction ()

function ( iwyu_target TARGET )
  if ( IWYU AND IWYU_PATH )
    get_target_property ( SOURCES ${TARGET} SOURCES )
    list ( FILTER SOURCES EXCLUDE REGEX "_export.h$" )
    add_library ( ${TARGET}_iwyu SHARED ${SOURCES} )
    set_target_properties ( ${TARGET}_iwyu PROPERTIES
                            CXX_COMPILER_LAUNCHER ${CMAKE_BINARY_DIR}/iwyu.sh
                            C_COMPILER_LAUNCHER ${CMAKE_BINARY_DIR}/iwyu.sh
                            EXCLUDE_FROM_ALL TRUE
                            OUTPUT_NAME ${TARGET}.iwyu.log
                            SUFFIX ""
                            PREFIX ""
                            LINK_DEPENDS ${CMAKE_BINARY_DIR}/iwyu_combine.sh
                            RULE_LAUNCH_LINK ${CMAKE_BINARY_DIR}/iwyu_combine.sh )
    copy_target_properties ( ${TARGET} _iwyu )
    set_source_files_properties ( ${SOURCES} PROPERTIES OBJECT_DEPENDS ${CMAKE_SOURCE_DIR}/iwyu.imp )
    target_include_directories ( ${TARGET}_iwyu SYSTEM PUBLIC ${CXX_DEFAULT_INCLUDES} )
    add_for_headers ( ${TARGET}_iwyu )
    add_dependencies ( iwyu ${TARGET}_iwyu )
    if ( DISABLE_COMPILE )
      set_target_properties ( ${TARGET} PROPERTIES
                              C_COMPILER_LAUNCHER ${CMAKE_SOURCE_DIR}/scripts/nulllinker.sh
                              CXX_COMPILER_LAUNCHER ${CMAKE_SOURCE_DIR}/scripts/nulllinker.sh
                              RULE_LAUNCH_LINK ${CMAKE_SOURCE_DIR}/scripts/nulllinker.sh )
    endif ()
  endif ()
endfunction ()
