add_library ( dvjson SHARED
        json.h
        JSONParser.cpp JSONParser.h
        JSONPath.cpp JSONPath.h
        JSONValue.cpp JSONValue.h
        jsonfwd.h
        JSONSerialiser.h
        JSONDiffListener.cpp JSONDiffListener.h
        JSONConverters.h
        JSONException.cpp JSONException.h
        JSONErrorCollector.cpp JSONErrorCollector.h
        JSONContext.h JSONContext.cpp )
target_link_libraries ( dvjson PUBLIC Boost::boost )
install ( TARGETS dvjson EXPORT dvjson DESTINATION ${CMAKE_INSTALL_LIBDIR} )
target_include_directories ( dvjson PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/dvformscpp/> )
iwyu_target ( dvjson )
clang_tidy_target ( dvjson )
