#include <gtest/gtest.h>
#include <gmock/gmock.h> // IWYU pragma: keep
#include <JSONPath.h>

using namespace testing;
using namespace dv::json;

class JSONPathTest : public ::testing::Test {
  void SetUp() override {
    RecordProperty( "COVERS", "JSONPath.cpp:JSONPath.h" );
  }
};

TEST_F( JSONPathTest, Test ) {
  JSONPath path, path2;

  EXPECT_EQ( "$", path.toString() );
  path /= "abc";
  EXPECT_EQ( "$.abc", path.toString() );
  EXPECT_EQ( "$.abc[1]", path[1].toString() );

  EXPECT_LT( path2, path );
  EXPECT_EQ( path, path );
  EXPECT_GT( path, path2 );
  EXPECT_LT( path[1], ( path2 / "abc" )[2] );
  EXPECT_LT( path[1], ( path2 / "abd" )[1] );
  EXPECT_EQ( path[1], path[1] );
  EXPECT_EQ( path / "x", path / "x" );
}
