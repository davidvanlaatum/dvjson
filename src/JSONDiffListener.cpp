#include "JSONDiffListener.h"
#include "JSONPath.h"  // for operator<<, JSONPath (ptr only)
#include <sstream>     // for operator<<, basic_ostream, endl
#include <utility>     // for pair

using namespace dv::json;

bool JSONDiffListenerImpl::isInterested() {
  return true;
}

void JSONDiffListenerImpl::recordDifference( const JSONPath &path, const std::string &diff ) {
  differences[path] = diff;
}

void JSONDiffListenerImpl::clear() {
  differences.clear();
}

std::string JSONDiffListenerImpl::toString() const {
  std::ostringstream str;
  str << *this;
  return str.str();
}

std::ostream &dv::json::operator<<( std::ostream &os, const JSONDiffListenerImpl &listener ) {
  for ( const auto &item : listener.differences ) {
    os << item.first << ": " << item.second << std::endl;
  }
  return os;
}
