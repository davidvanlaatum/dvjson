#pragma once
#ifndef DVMON_GTESTJSON_H
#define DVMON_GTESTJSON_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "json.h"

namespace dv {
  namespace json {
    namespace testing {
      template<typename T> class JsonEqualsMatcher : public ::testing::MatcherInterface<T> {
      public:
        template<typename JSONType=JSON, detail::enable_if_t<std::is_same<JSON, JSONType>::value> = 0> JsonEqualsMatcher( const JSONType &nexpected )
          : expected( nexpected ) {}

        JsonEqualsMatcher( const std::string &json ) {
          JSONParser parser;
          parser.parseInto( expected, json );
        }

        void DescribeTo( ::std::ostream *os ) const override {
          *os << "is equal to " << expected;
        }

        void DescribeNegationTo( ::std::ostream *os ) const override {
          *os << "is not equal to " << expected;
        }

        bool MatchAndExplain( T x, ::testing::MatchResultListener *listener ) const override {
          JSON other;
          JSONParser parser;
          parser.parseInto( other, x );
          JSONDiffListenerImpl diffListener;
          bool rt = expected.compare( other, diffListener );
          if ( !rt && listener->IsInterested() ) {
            *listener << diffListener;
          }
          return rt;
        }

      protected:
        JSON expected;
      };

      template<typename T, typename Y> inline ::testing::Matcher<T> jsonEquals( Y value ) {
        return MakeMatcher( new JsonEqualsMatcher<T>( value ) );
      }
    }
  }
}

#endif //DVMON_GTESTJSON_H
