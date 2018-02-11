#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <JSONObject.h>
#include <json.h>

using namespace testing;
using namespace dv::json;

class JSONObjectTest : public ::testing::Test {
  void SetUp() override {
    RecordProperty( "COVERS", "JSONObject.cpp:JSONObject.h" );
  }
};

TEST_F( JSONObjectTest, Test ) {
  JSONObject obj;

  *obj["abc"] = 1;
  ASSERT_THAT( obj, SizeIs( 1 ) );
  *obj["def"] = 2;
  ASSERT_THAT( obj, SizeIs( 2 ) );
  *obj["hij"] = 3;
  ASSERT_THAT( obj, SizeIs( 3 ) );

  for ( const auto &item : obj ) {
    ASSERT_LT( item.second->as<int>(), 4 );
  }
}
