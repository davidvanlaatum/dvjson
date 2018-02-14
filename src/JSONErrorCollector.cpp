#include "JSONErrorCollector.h"
#include "JSONException.h"
#include "JSONPath.h"

using namespace dv::json;

JSONErrorCollector::~JSONErrorCollector() = default;

JSONErrorCollectorThrow::~JSONErrorCollectorThrow() = default;

void JSONErrorCollectorThrow::error( const JSONPath &path, const std::string &message ) {
  throw JSONException( path.toString() + ": " + message );
}

JSONErrorCollectorImpl::~JSONErrorCollectorImpl() = default;

void JSONErrorCollectorImpl::error( const JSONPath &path, const std::string &message ) {
  messages.emplace_back( path.toString() + ": " + message );
}

bool JSONErrorCollectorImpl::empty() const {
  return messages.empty();
}

const std::list<std::string> &JSONErrorCollectorImpl::getMessages() const {
  return messages;
}
