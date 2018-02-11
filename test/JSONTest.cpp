#include <iosfwd>         // for ostringstream
#include "json.h"         // for JSON, JSONDiffListenerImpl, operator<<
#include <gtest/gtest.h>
#include <gmock/gmock.h>  // IWYU pragma: keep
#include <JSONPath.h>     // IWYU pragma: keep

using namespace testing;
using namespace dv::json;

class JSONTest : public ::testing::Test {
  void SetUp() override {
    RecordProperty( "COVERS", "JSON.cpp:JSON.h" );
  }
};

TEST_F( JSONTest, Dump ) {
  JSON json;

  json["hi all"] = 123;
  std::ostringstream str;
  json.dump( str, 0 );
  EXPECT_EQ( str.str(), "{\"hi all\":123}" );

  str.str( "" );
  json[1] = 1;
  json.dump( str, 0 );
  EXPECT_EQ( str.str(), "[null,1]" );

  str.str( "" );
  json = 1;
  json.dump( str, 0 );
  EXPECT_EQ( str.str(), "1" );

  str.str( "" );
  json = 1.1;
  json.dump( str, 0 );
  EXPECT_EQ( str.str(), "1.1" );

  str.str( "" );
  json = true;
  json.dump( str, 0 );
  EXPECT_EQ( str.str(), "true" );

  str.str( "" );
  json = false;
  json.dump( str, 0 );
  EXPECT_EQ( str.str(), "false" );

  str.str( "" );
  json = "hi \"all\"";
  json.dump( str, 0 );
  EXPECT_EQ( str.str(), "\"hi \\\"all\\\"\"" );
}

TEST_F( JSONTest, Compare ) {
  JSON json1, json2;

  JSONDiffListenerImpl listener;

  EXPECT_TRUE( json1.compare( json2, listener ) ) << listener;
  listener.clear();

  json1["abc"] = nullptr;
  json2["xyz"] = nullptr;
  EXPECT_FALSE( json1.compare( json2, listener ) );
  EXPECT_EQ( "$.abc: Missing from right\n$.xyz: Missing from left\n", listener.toString() );
  listener.clear();

  json1 = 1;
  json2 = 1;
  EXPECT_TRUE( json1.compare( json2, listener ) ) << listener;
  listener.clear();

  json1 = 1;
  json2 = 1.0;
  EXPECT_FALSE( json1.compare( json2, listener ) );
  EXPECT_EQ( "$: Type mismatch int != double\n", listener.toString() );
  listener.clear();

  json1 = 1;
  json2 = 2;
  EXPECT_FALSE( json1.compare( json2, listener ) );
  EXPECT_EQ( "$: 1 != 2\n", listener.toString() );
  listener.clear();
}
