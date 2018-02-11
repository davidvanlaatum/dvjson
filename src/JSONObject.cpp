#include "JSONObject.h"
#include "JSONValue.h"

using namespace dv::json;

JSONObject::const_iterator JSONObject::begin() const {
  return container.begin();
}

JSONObject::const_iterator JSONObject::end() const {
  return container.end();
}

JSONObject::size_type JSONObject::size() const {
  return container.size();
}

bool JSONObject::empty() const {
  return container.empty();
}

JSONObject::findIterator JSONObject::find( const JSONObject::keyType &key ) const {
  const auto &item = index.find( key );
  return JSONObject::findIterator( item != index.end() ? &container[item->second] : nullptr );
}

JSONPtr &JSONObject::operator[]( const JSONObject::keyType &key ) {
  const auto &item = index.find( key );
  if ( item != index.end() ) {
    return container[item->second].second;
  } else {
    const auto &i = container.emplace( container.end(), key, std::make_shared<JSON>() );
    index.emplace( i->first, container.size() - 1 );
    assert( container.size() == index.size() );
    return i->second;
  }
}

bool JSONObject::operator==( const JSONObject &other ) const {
  return this == &other;
}

bool JSONObject::operator!=( const JSONObject &other ) const {
  return this != &other;
}

JSONObject &JSONObject::operator=( const JSONObject & ) {
  index.clear();
  container.clear();

  return *this;
}

JSONObject::findIterator::findIterator( const JSONObject::valueType *ptr ) : ptr( ptr ) {}

const JSONObject::valueType *JSONObject::findIterator::operator->() const {
  return ptr;
}

bool JSONObject::findIterator::operator==( const JSONObject::const_iterator &other ) const {
  return ptr == nullptr;
}

bool JSONObject::findIterator::operator!=( const JSONObject::const_iterator &other ) const {
  return ptr != nullptr;
}
