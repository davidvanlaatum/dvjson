#pragma once
#ifndef DVJSON_JSONPARSER_H
#define DVJSON_JSONPARSER_H

#include "jsonfwd.h"
#include "JSONException.h"

namespace dv {
  namespace json {
    class JSON;
    class JSONParser {
     public:
      void parseInto( JSON &value, std::istream &stream );
      void parseInto( JSON &value, const std::string &string );
    };
  }
}

#endif //DVJSON_JSONPARSER_H
