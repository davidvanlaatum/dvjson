#pragma once
#ifndef DVFORMSCPP_JSONPARSER_H
#define DVFORMSCPP_JSONPARSER_H

#include <iosfwd>
#include <string>
#include <stdexcept>

namespace dv {
  namespace json {
    class JSONParseException : public std::exception {
     public:
      explicit JSONParseException( const std::string &nMessage );
      const char *what() const noexcept override;
     protected:
      std::string message;
    };

    class JSON;
    class JSONParser {
     public:
      void parseInto( JSON &value, std::istream &stream );
      void parseInto( JSON &value, const std::string &string );
    };
  }
}

#endif //DVFORMSCPP_JSONPARSER_H
