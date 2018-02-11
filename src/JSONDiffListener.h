#pragma once
#ifndef DVFORMSCPP_JSONDIFFLISTENER_H
#define DVFORMSCPP_JSONDIFFLISTENER_H

#include <iosfwd>  // for ostream
#include <map>     // for map
#include <string>  // for string
#include <JSONPath.h>

namespace dv {
  namespace json {
    class JSONDiffListener {
     public:
      virtual ~JSONDiffListener() = default;
      virtual bool isInterested() = 0;
      virtual void recordDifference( const JSONPath &path, const std::string &diff ) = 0;
    };

    class JSONDiffNullListener : public JSONDiffListener {
     public:
      ~JSONDiffNullListener() override = default;

      bool isInterested() override { return false; }

      void recordDifference( const JSONPath &/*path*/, const std::string &/*compare*/ ) override {}
    };

    class JSONDiffListenerImpl : public JSONDiffListener {
     public:
      ~JSONDiffListenerImpl() override = default;
      bool isInterested() override;
      void recordDifference( const JSONPath &path, const std::string &diff ) override;
      void clear();
      std::string toString() const;
     protected:
      std::map<JSONPath, std::string> differences;
      friend std::ostream &operator<<( std::ostream &os, const JSONDiffListenerImpl &listener );
    };
  }
}

#endif //DVFORMSCPP_JSONDIFFLISTENER_H
