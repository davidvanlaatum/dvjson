#include "JSONParser.h"
#include "json.h"                      // for JSON, JSONPtr, JSONTypes::doubleType, Type, JSONTypes::intType, Type::ARRAY, Type::INT, Type::OBJECT
#include <boost/algorithm/string.hpp>  // for iequals
#include <cctype>                      // for isspace, isdigit
#include <cstddef>                     // for size_t
#include <list>                        // for _List_iterator, list
#include <memory>                      // for __shared_ptr_access, __shared_ptr_access<>::element_type, make_shared, shared_ptr
#include <sstream>                     // for operator<<, basic_istream::get, basic_ostream, basic_istream::unget, basic_istream::readsome, basic_istream::peek
#include <vector>                      // for vector

using namespace dv::json;

void JSONParser::parseInto( JSON &value, const std::string &string ) {
  std::istringstream stream( string );
  parseInto( value, stream );
}

enum JSONParserState {
  UNKNOWN,
  STRING,
  INTEGER,
  DOUBLE,
  NULLVALUE,
  ARRAY,
  OBJECTKEY,
  OBJECTVALUE
};

struct JSONParserStackItem {
  JSONParserState state{ UNKNOWN };
  JSONPtr node;
  std::string buffer;
  bool escape{ false };
  bool negative{ false };
  bool inName{ false };
  size_t decimalPlace{ 0 };

  bool hasContext() const {
    return !buffer.empty();
  }
};

struct JSONParserStack : std::list<JSONParserStackItem> {
  JSONParserStackItem *push( const JSONPtr &node = JSONPtr() ) {
    emplace_back();
    auto rt = rbegin();
    rt->node = node ? node : std::make_shared<JSON>();
    return &*rt;
  }

  JSONParserStackItem *pop() {
    if ( empty() ) {
      throw JSONParseException( "Tried to pop empty stack" );
    }
    pop_back();
    auto rt = rbegin();
    return rt == rend() ? nullptr : &*rt;
  }
};

void JSONParser::parseInto( JSON &value, std::istream &stream ) {
  static const std::string nullstr( "null" );
  static const std::string truestr( "true" );
  static const std::string falsestr( "false" );
  JSONParserStack stack;
  JSONParserStackItem *stackptr = stack.push();
  auto newValue = stackptr->node;
  try {
    auto c = static_cast<char>(stream.get());
    while ( stream.good() && stackptr ) {
      if ( stackptr->state == UNKNOWN ) {
        if ( c == '"' ) {
          stackptr->state = STRING;
          c = static_cast<char>(stream.get());
        } else if ( isdigit( c ) ) {
          stackptr->state = INTEGER;
        } else if ( ( ( c == '-' || c == '+' ) && isdigit( stream.peek() ) ) ) {
          stackptr->state = INTEGER;
          stackptr->negative = c == '-';
          c = static_cast<char>(stream.get());
        } else if ( c == '[' ) {
          stackptr->state = ARRAY;
          *stackptr->node = Type::ARRAY;
        } else if ( c == '{' ) {
          stackptr->state = OBJECTKEY;
          *stackptr->node = Type::OBJECT;
          c = static_cast<char>(stream.get());
        } else if ( stackptr->buffer.size() > 32 ) {
          throw JSONParseException( "Syntax error" );
        }
        if ( stackptr->state != UNKNOWN && stackptr->hasContext() ) {
          throw JSONParseException( "Syntax error" );
        }
      }
      if ( stream.good() ) {
        switch ( stackptr->state ) {
          case UNKNOWN:
            if ( !isspace( c ) || !stackptr->buffer.empty() ) {
              stackptr->buffer += c;
              if ( ( c == 'l' || c == 'L' ) && boost::iequals( stackptr->buffer, nullstr ) ) {
                stackptr->state = NULLVALUE;
                stackptr->buffer.clear();
                continue;
              } else if ( ( c == 'e' || c == 'E' ) ) {
                if ( boost::iequals( stackptr->buffer, truestr ) ) {
                  *stackptr->node = true;
                  stackptr = stack.pop();
                } else if ( boost::iequals( stackptr->buffer, falsestr ) ) {
                  *stackptr->node = false;
                  stackptr = stack.pop();
                }
              }
            }
            break;
          case STRING:
            if ( !stackptr->escape && c == '\\' ) {
              stackptr->escape = true;
            } else if ( stackptr->escape || c != '"' ) {
              stackptr->buffer += c;
              stackptr->escape = false;
            } else /*if ( c == '"' )*/ {
              *stackptr->node = stackptr->buffer;
              stackptr = stack.pop();
            }
            break;
          case INTEGER:
            if ( c == '.' ) {
              stackptr->state = DOUBLE;
              stackptr->decimalPlace = 10;
              *stackptr->node = stackptr->node->as<JSON::doubleType>();
            } else if ( isdigit( c ) ) {
              JSON::intType x = c - '0', y = 0;
              if ( stackptr->node->type() == Type::INT ) {
                y = stackptr->node->as<JSON::intType>();
                y *= 10;
              }
              if ( stackptr->negative ) {
                y -= x;
              } else {
                y += x;
              }
              *stackptr->node = y;
            } else {
              stackptr = stack.pop();
              continue;
            }
            break;
          case DOUBLE:
            if ( isdigit( c ) ) {
              JSON::doubleType x = c - '0';
              auto y = stackptr->node->as<JSON::doubleType>();
              if ( stackptr->negative ) {
                y -= x / static_cast<JSON::doubleType>(stackptr->decimalPlace);
              } else {
                y += x / static_cast<JSON::doubleType>(stackptr->decimalPlace);
              }
              stackptr->decimalPlace *= 10;
              *stackptr->node = y;
            } else {
              stackptr = stack.pop();
              continue;
            }
            break;
          case NULLVALUE:
            *stackptr->node = nullptr;
            stackptr = stack.pop();
            break;
          case ARRAY:
            if ( c == '[' ) {
              *stackptr->node = Type::ARRAY;
              stackptr = stack.push( stackptr->node->emplaceBack( std::make_shared<JSON>() ) );
            } else if ( c == ',' ) {
              stackptr = stack.push( stackptr->node->emplaceBack( std::make_shared<JSON>() ) );
            } else if ( c == ']' ) {
              stackptr = stack.pop();
            } else if ( !isspace( c ) ) {
              stream.unget();
              throw JSONParseException( "Syntax error" );
            }
            break;
          case OBJECTKEY:
            if ( !isspace( c ) ) {
              if ( stackptr->inName && c == '\\' ) {
                stackptr->escape = true;
              } else if ( c == '"' && !stackptr->escape ) {
                stackptr->inName = !stackptr->inName;
              } else if ( stackptr->inName ) {
                stackptr->buffer += c;
                stackptr->escape = false;
              } else if ( !stackptr->inName && c == ':' ) {
                stackptr->state = OBJECTVALUE;
                auto name = stackptr->buffer;
                stackptr->buffer.clear();
                stackptr = stack.push( ( *stackptr->node )[name].shared_from_this() );
              } else if ( c == '}' ) {
                stackptr->state = OBJECTVALUE;
                continue;
              } else {
                stream.unget();
                throw JSONParseException( "Syntax error" );
              }
            }
            break;
          case OBJECTVALUE:
            if ( !isspace( c ) ) {
              if ( c == ',' ) {
                stackptr->state = OBJECTKEY;
              } else if ( c == '}' ) {
                stackptr = stack.pop();
              } else {
                stream.unget();
                throw JSONParseException( "Syntax error" );
              }
            }
            break;
        }
      }
      c = static_cast<char>(stream.get());
    }
    if ( stackptr && !stream.good() ) {
      if ( stackptr->hasContext() ) {
        throw JSONParseException( "Syntax error" );
      }
      stackptr = stack.pop();
    }
  } catch ( JSONParseException &e ) {
    std::ostringstream msg;
    msg << e.what() << " at '";
    if ( stackptr && stackptr->hasContext() ) {
      msg << stackptr->buffer;
    }
    char buffer[129] = { 0 };
    if ( stream.readsome( buffer, sizeof( buffer ) - 1 ) > 0 ) {
      msg << buffer;
    }
    msg << "'";
    throw JSONParseException( msg.str() );
  }
  if ( stackptr ) {
    throw JSONParseException( "Early exit from loop at '" + stackptr->buffer + "'" );
  } else if ( stream.good() ) {
    std::vector<char> buffer( 129 );
    if ( stream.readsome( buffer.data(), static_cast<std::streamsize>(buffer.size() - 1) ) > 0 ) {
      auto ptr = buffer.begin();
      while ( ptr != buffer.end() && isspace( *ptr ) ) { ptr++; }
      if ( ptr != buffer.end() && *ptr != 0 ) {
        buffer.erase( buffer.begin(), ptr );
        throw JSONParseException( std::string( "Early exit from loop at '" ) + buffer.data() + "'" );
      }
    }
  }

  value = *newValue;
}

JSONParseException::JSONParseException( const std::string &nMessage ) : message( nMessage ) {}

const char *JSONParseException::what() const noexcept {
  return message.c_str();
}
