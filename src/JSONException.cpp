#include "JSONException.h"

using namespace dv::json;

JSONException::JSONException() = default;

JSONException::JSONException( const std::string &message ) : msg( message ) {}

JSONException::JSONException( const char *message ) : msg( message ) {}

JSONException::~JSONException() = default;

const char *JSONException::what() const noexcept {
  return msg.c_str();
}

JSONParseException::JSONParseException( const std::string &nMessage ) : JSONException( nMessage ) {}

JSONParseException::JSONParseException( const char *nMessage ) : JSONException( nMessage ) {}

JSONWrongTypeException::JSONWrongTypeException( const std::type_info &nExpected, const std::type_info &nActual ) : expected( nExpected ), actual( nActual ) {}

const std::type_info &JSONWrongTypeException::getExpected() const {
  return expected;
}

const std::type_info &JSONWrongTypeException::getActual() const {
  return actual;
}

JSONNoSuchKeyException::JSONNoSuchKeyException( const char *nKey ) : JSONException( "no such key: " + std::string( nKey ) ), key( nKey ) {}

JSONNoSuchKeyException::JSONNoSuchKeyException( const std::string &nKey ) : JSONException( "no such key: " + nKey ), key( nKey ) {}

JSONNoSuchKeyException::JSONNoSuchKeyException( std::size_t index ) : JSONException( "No such index: " + std::to_string( index ) ),
                                                                      key( std::to_string( index ) ) {}
