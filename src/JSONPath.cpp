#include "JSONPath.h"
#include "json.h"
#include <boost/variant.hpp>      // for type_info
#include <boost/variant/get.hpp>  // for get
#include <sstream>

using namespace dv::json;

dv::json::JSONPath dv::json::JSONPath::operator/( const dv::json::JSON::stringType &name ) const {
  dv::json::JSONPath rt( *this );
  rt.path.emplace_back( name );
  return rt;
}

dv::json::JSONPath dv::json::JSONPath::operator[]( size_t index ) const {
  dv::json::JSONPath rt( *this );
  rt.path.emplace_back( index );
  return rt;
}

std::string dv::json::JSONPath::toString() const {
  std::ostringstream str;

  str << "$";

  for ( const auto &item : path ) {
    if ( item.type() == typeid( JSON::stringType ) ) {
      str << "." << boost::get<JSON::stringType>( item );
    } else {
      str << "[" << boost::get<size_t>( item ) << "]";
    }
  }

  return str.str();
}

dv::json::JSONPath dv::json::JSONPath::operator/=( const dv::json::JSON::stringType &name ) {
  *this = *this / name;
  return *this;
}

bool dv::json::JSONPath::operator<( const dv::json::JSONPath &other ) const {
  return compare( other ) < 0;
}

bool dv::json::JSONPath::operator>( const dv::json::JSONPath &other ) const {
  return compare( other ) > 0;
}

bool dv::json::JSONPath::operator<=( const dv::json::JSONPath &other ) const {
  return compare( other ) <= 0;
}

bool dv::json::JSONPath::operator>=( const dv::json::JSONPath &other ) const {
  return compare( other ) >= 0;
}

bool dv::json::JSONPath::operator==( const dv::json::JSONPath &other ) const {
  return compare( other ) == 0;
}

int dv::json::JSONPath::compare( const dv::json::JSONPath &other ) const {
  int rt = 0;
  auto it1 = path.begin(), it2 = other.path.begin();

  while ( it1 != path.end() && it2 != other.path.end() ) {
    if ( it1->type() != it2->type() ) {
      if ( it1->type() == typeid( JSON::stringType ) ) {
        rt = -1;
      } else {
        rt = 1;
      }
      break;
    } else if ( it1->type() == typeid( JSON::stringType ) ) {
      auto x = boost::get<JSON::stringType>( *it1 ).compare( boost::get<JSON::stringType>( *it2 ) );
      if ( x != 0 ) {
        rt = x > 0 ? 1 : -1;
        break;
      }
    } else {
      auto i1 = boost::get<size_t>( *it1 ), i2 = boost::get<size_t>( *it2 );
      if ( i1 != i2 ) {
        rt = i1 > i2 ? 1 : -1;
        break;
      }
    }
    ++it1;
    ++it2;
  }

  if ( rt == 0 ) {
    if ( it1 != path.end() ) {
      rt = 1;
    } else if ( it2 != other.path.end() ) {
      rt = -1;
    }
  }

  return rt;
}
