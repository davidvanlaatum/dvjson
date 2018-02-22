#include "JSONValue.h"
#include "JSONDiffListener.h"  // for JSONDiffListener, JSONDiffNullListener
#include "JSONPath.h"          // for JSONPath
#include "JSONConverters.h"
#include "JSONException.h"
#include "JSONParser.h"
#include <set>
#include <sstream>
#include <string>              // for string

using namespace dv::json;

void detail::writeJSON( std::ostream &os, const JSON &json ) {
  json.dump( os, 2 );
}

void detail::readJSON( std::istream &is, JSON &json ) {
  JSONParser parser;
  parser.parseInto( json, is );
}

JSON::reference JSON::operator[]( const JSON::keyType &key ) {
  if ( value.type() != typeid( objectType ) ) {
    value = objectType();
  }
  auto &rt = boost::get<objectType>( value )[key];
  if ( !rt ) {
    rt = std::make_shared<JSON>();
  }
  return *rt;
}

JSON::const_reference JSON::operator[]( const JSON::keyType &key ) const {
  if ( value.type() != typeid( objectType ) ) {
    throw JSONWrongTypeException( typeid( objectType ), value.type() );
  }
  auto &rt = boost::get<objectType>( value )[key];
  if ( !rt ) {
    throw JSONNoSuchKeyException( key );
  }
  return *rt;
}

JSON::reference JSON::operator[]( JSON::indexType index ) {
  if ( value.type() != typeid( arrayType ) ) {
    value = arrayType();
  }
  auto &vector = boost::get<arrayType>( value );
  while ( vector.size() <= index ) {
    vector.emplace_back( std::make_shared<JSON>() );
  }
  auto &rt = vector.at( index );
  if ( !rt ) {
    vector[index] = rt = std::make_shared<JSON>();
  }
  return *rt;
}

JSON::const_reference JSON::operator[]( JSON::indexType index ) const {
  if ( value.type() != typeid( objectType ) ) {
    throw JSONWrongTypeException( typeid( objectType ), value.type() );
  }
  auto &vector = boost::get<arrayType>( value );
  if ( vector.size() < index ) {
    throw JSONNoSuchKeyException( index );
  }
  return *vector[index];
}

bool JSON::compare( const JSON &other, JSONDiffListener &listener ) const {
  JSONPath path;
  return compare( other, listener, path );
}

bool JSON::compare( const JSON &other, JSONDiffListener &listener, const JSONPath &path ) const {
  bool rt = false;
  auto t1 = type();
  auto t2 = other.type();
  if ( t1 == t2 ) {
    switch ( t1 ) {
      case Type::STRING:
      case Type::DOUBLE:
      case Type::BOOL:
      case Type::INT:
        rt = value == other.value;
        if ( !rt && listener.isInterested() ) {
          std::ostringstream msg;
          msg << *this << " != " << other;
          listener.recordDifference( path, msg.str() );
        }
        break;
      case Type::NULLVALUE:
        rt = true;
        break;
      case Type::OBJECT: {
        std::set<stringType> keys1, keys2;
        auto obj1 = boost::get<objectType>( value );
        auto obj2 = boost::get<objectType>( other.value );

        for ( const auto &item : obj1 ) {
          keys1.insert( item.first );
        }
        for ( const auto &item : obj2 ) {
          keys2.insert( item.first );
        }

        if ( !listener.isInterested() ) {
          if ( keys1 != keys2 ) {
            break;
          }
        }
        rt = true;
        auto it1 = keys1.begin(), it2 = keys2.begin();
        while ( it1 != keys1.end() && it2 != keys2.end() ) {
          if ( *it1 == *it2 ) {

            if ( !obj1.find( *it1 )->second->compare( *obj2.find( *it2 )->second, listener, path / *it1 ) ) {
              rt = false;
              if ( !listener.isInterested() ) {
                it1 = keys1.end();
                it2 = keys2.end();
                break;
              }
            }
            ++it1;
            ++it2;
          } else if ( *it1 < *it2 ) {
            rt = false;
            auto x = it1;
            while ( x != keys1.end() && *x < *it2 ) { ++x; }
            if ( x == keys1.end() ) {
              listener.recordDifference( path / *it2, "Missing from left" );
              ++it2;
            } else {
              while ( it1 != x ) {
                listener.recordDifference( path / *it1, "Missing from right" );
                ++it1;
              }
            }
          } else {
            rt = false;
            auto x = it2;
            while ( x != keys2.end() && *x < *it1 ) { ++x; }
            if ( x == keys2.end() ) {
              listener.recordDifference( path / *it1, "Missing from right" );
              ++it1;
            } else {
              while ( it2 != x ) {
                listener.recordDifference( path / *it2, "Missing from left" );
                ++it2;
              }
            }
          }
        }
        while ( it1 != keys1.end() ) {
          rt = false;
          listener.recordDifference( path / *it1, "Missing from right" );
          ++it1;
        }
        while ( it2 != keys2.end() ) {
          rt = false;
          listener.recordDifference( path / *it2, "Missing from left" );
          ++it2;
        }
        break;
      }
      case Type::ARRAY: {
        auto arr1 = boost::get<arrayType>( value );
        auto arr2 = boost::get<arrayType>( other.value );
        auto it1 = arr1.begin();
        auto it2 = arr2.begin();
        size_t index1 = 0;
        size_t index2 = 0;
        rt = true;
        while ( it1 != arr1.end() && it2 != arr2.end() ) {
          if ( !( *it1 )->compare( **it2, listener, path[index1] ) ) {
            rt = false;
          }
          ++it1;
          index1++;
          ++it2;
          index2++;
        }
        if ( it1 != arr1.end() || it2 != arr2.end() ) {
          rt = false;
          if ( listener.isInterested() ) {
            while ( it1 != arr1.end() ) {
              listener.recordDifference( path[index1], "Missing from right" );
              ++it1;
              index1++;
            }
            while ( it2 != arr2.end() ) {
              listener.recordDifference( path[index2], "Missing from left" );
              ++it2;
              index2++;
            }
          }
        }
        break;
      }
    }
  } else if ( listener.isInterested() ) {
    std::ostringstream msg;
    msg << "Type mismatch " << t1 << " != " << t2;
    listener.recordDifference( path, msg.str() );
  }
  return rt;
}

bool JSON::operator==( const JSON &other ) const {
  JSONDiffNullListener listener;
  return compare( other, listener );
}

bool JSON::operator!=( const JSON &other ) const {
  JSONDiffNullListener listener;
  return !compare( other, listener );
}

class JSONTypeVisitor : public boost::static_visitor<Type> {
public:
#define TYPE( t, T ) Type operator()( const t & ) { return Type::T; }

  TYPE( JSON::nullType, NULLVALUE )

  TYPE( JSON::boolType, BOOL )

  TYPE( JSON::intType, INT )

  TYPE( JSON::doubleType, DOUBLE )

  TYPE( JSON::stringType, STRING )

  TYPE( JSON::objectType, OBJECT )

  TYPE( JSON::arrayType, ARRAY )
};

Type JSON::type() const noexcept {
  JSONTypeVisitor visitor;
  return value.apply_visitor( visitor );
}

JSON &JSON::operator=( Type v ) {
  switch ( v ) {
    case Type::ARRAY:
      *this = arrayType();
      break;
    case Type::DOUBLE:
      *this = static_cast<doubleType>(0.0);
      break;
    case Type::INT:
      *this = static_cast<intType>(0);
      break;
    case Type::NULLVALUE:
      *this = nullptr;
      break;
    case Type::STRING:
      *this = stringType();
      break;
    case Type::OBJECT:
      *this = objectType();
      break;
    case Type::BOOL:
      *this = false;
      break;
  }
  return *this;
}

JSON &JSON::operator=( const JSON &other ) {
  value = other.value;
  return *this;
}

JSON &JSON::operator=( JSON &&other ) {
  value = std::move( other.value );
  return *this;
}

void JSON::writeEscaped( std::ostream &os, const JSON::stringType &v ) const {
  for ( auto item : v ) {
    if ( item == '"' ) {
      os << "\\\"";
    } else {
      os << item;
    }
  }
}

void JSON::dump( std::ostream &os, unsigned int indent ) const {
  dump( os, indent, 1 );
}

static std::string doIndent( unsigned int indent, unsigned int level ) noexcept JSON_PURE;

static std::string doIndent( unsigned int indent, unsigned int level ) noexcept {
  std::string rt;

  if ( indent * level > 0 ) {
    rt = "\n";
    for ( unsigned int i = 0; i < ( level - 1 ) * indent; i++ ) {
      rt += " ";
    }
  }

  return rt;
}

void JSON::dump( std::ostream &os, unsigned int indent, unsigned int level ) const {
  level++;
  switch ( type() ) {
    case Type::NULLVALUE:
      os << "null";
      break;
    case Type::OBJECT: {
      const auto &obj = boost::get<objectType>( value );
      if ( obj.empty() ) {
        os << "{}";
      } else {
        os << "{" << doIndent( indent, level );
        bool first = true;
        for ( const auto &it : obj ) {
          if ( !first ) { os << "," << doIndent( indent, level ); }
          os << "\"";
          writeEscaped( os, it.first );
          os << "\":";
          if ( indent > 0 ) { os << " "; }
          it.second->dump( os, indent, level );
          first = false;
        }
        os << doIndent( indent, level - 1 ) << "}";
      }
      break;
    }
    case Type::STRING:
      os << "\"";
      writeEscaped( os, boost::get<stringType>( value ) );
      os << "\"";
      break;
    case Type::BOOL:
      os << ( boost::get<boolType>( value ) ? "true" : "false" );
      break;
    case Type::INT:
      os << boost::get<intType>( value );
      break;
    case Type::DOUBLE:
      os << boost::get<doubleType>( value );
      break;
    case Type::ARRAY: {
      const auto &list = boost::get<arrayType>( value );
      if ( list.empty() ) {
        os << "[]";
      } else {
        os << "[";
        bool first = true;
        for ( const auto &it : list ) {
          if ( !first ) { os << "," << doIndent( indent, level ); }
          first = false;
          it->dump( os, indent, level );
        }
        os << doIndent( indent, level - 1 ) << "]";
      }
      break;
    }
  }
}

bool JSON::operator==( Type t ) const noexcept {
  return type() == t;
}

bool JSON::operator!=( Type t ) const noexcept {
  return type() != t;
}

std::string JSON::toString() const {
  std::ostringstream str;
  str << *this;
  return str.str();
}

std::ostream &dv::json::operator<<( std::ostream &os, Type type ) {
  switch ( type ) {
    case Type::INT:
      os << "int";
      break;
    case Type::BOOL:
      os << "bool";
      break;
    case Type::DOUBLE:
      os << "double";
      break;
    case Type::STRING:
      os << "string";
      break;
    case Type::OBJECT:
      os << "object";
      break;
    case Type::ARRAY:
      os << "array";
      break;
    case Type::NULLVALUE:
      os << "null";
      break;
  }
  return os;
}
