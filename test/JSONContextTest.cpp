#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <JSONContext.h>
#include <JSONErrorCollector.h>

using namespace testing;

class JSONContextTest : public ::testing::Test {
  void SetUp() override {
    RecordProperty( "COVERS", "JSONContext.h" );
  }
};

TEST_F( JSONContextTest, Test ) {
  dv::json::JSONContext context;

  auto ptr = new char;

  context.attach( ptr );

  ASSERT_EQ( ptr, context.get<char>().get() );
  ASSERT_FALSE( context.get<int>() );

  int i = 0;
  context.attach( &i, false );

  ASSERT_TRUE( context.get<dv::json::JSONErrorCollector>() );
}
