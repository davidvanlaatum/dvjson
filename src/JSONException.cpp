#include "JSONException.h"

using namespace dv::json;

JSONException::JSONException() = default;

JSONException::JSONException( const std::string &message ) : msg( message ) {}

JSONException::JSONException( const char *message ) : msg( message ) {}

JSONException::~JSONException() = default;

const char *JSONException::what() const noexcept {
  return msg.c_str();
}
