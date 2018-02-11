#include <gtest/gtest.h>
#include <gmock/gmock.h>  // IWYU pragma: keep
#include <cstdint>        // for int64_t
#include <vector>         // for vector
#include "json.h"         // for JSON, Type, to_json, Type::INT, Type::BOOL, Type::DOUBLE, Type::OBJECT, Type::ARRAY, Type::NULLVALUE, Type::STRING
#include <JSONParser.h>

using namespace testing;
using namespace dv::json;

class JSONParserTest : public ::testing::Test {
  void SetUp() override {
    RecordProperty( "COVERS", "JSONParser.cpp:JSONParser.h" );
  }
};

#define EXPECT_THROW_WITH_MESSAGE( code, message ) \
try { \
    code; \
    FAIL() << "Expected to throw"; \
} catch ( JSONParseException &e ) { \
    EXPECT_STREQ(e.what(),message);\
}

TEST_F( JSONParserTest, Test ) {
  JSONParser parser;

  JSON value;
  EXPECT_NO_THROW( parser.parseInto( value, "null" ) );
  EXPECT_EQ( value, Type::NULLVALUE );
  EXPECT_NO_THROW( parser.parseInto( value, "\"hi all\"" ) );
  EXPECT_EQ( value, Type::STRING );
  EXPECT_EQ( value, "hi all" );
  EXPECT_NO_THROW( parser.parseInto( value, "123" ) );
  EXPECT_EQ( value, Type::INT );
  EXPECT_EQ( value, 123 );
  EXPECT_NO_THROW( parser.parseInto( value, "+123" ) );
  EXPECT_EQ( value, Type::INT );
  EXPECT_EQ( value, 123 );
  EXPECT_NO_THROW( parser.parseInto( value, "-123" ) );
  EXPECT_EQ( value, Type::INT );
  EXPECT_EQ( value, -123 );
  EXPECT_NO_THROW( parser.parseInto( value, "123.456" ) );
  EXPECT_EQ( value, Type::DOUBLE );
  EXPECT_EQ( value, 123.456 );
  EXPECT_NO_THROW( parser.parseInto( value, "-123.456" ) );
  EXPECT_EQ( value, Type::DOUBLE );
  EXPECT_EQ( value, -123.456 );

  EXPECT_NO_THROW( parser.parseInto( value, "[1,2 ,3 ]" ) );
  EXPECT_EQ( value, Type::ARRAY );
  EXPECT_EQ( value, std::vector<int64_t>( { 1, 2, 3 } ) );

  EXPECT_NO_THROW( parser.parseInto( value, "true" ) );
  EXPECT_EQ( value, Type::BOOL );
  EXPECT_EQ( value, true );

  EXPECT_NO_THROW( parser.parseInto( value, "false" ) );
  EXPECT_EQ( value, Type::BOOL );
  EXPECT_EQ( value, false );

  JSON expected;
  expected["abc"] = 123;
  expected["def"] = 456.789;
  expected["hij"] = "klm";

  value = nullptr;
  EXPECT_NO_THROW( parser.parseInto( value, "{ \"abc\": 123, \"def\": 456.789, \"hij\": \"klm\" }" ) );
  EXPECT_EQ( value, Type::OBJECT );
  EXPECT_EQ( value, expected );

  value = nullptr;
  EXPECT_NO_THROW( parser.parseInto( value, "  {\n}  \n " ) );
  EXPECT_EQ( value, Type::OBJECT );

  value = nullptr;
  EXPECT_THROW_WITH_MESSAGE( parser.parseInto( value, "[123 null]" ), "Syntax error at 'null]'" );
  EXPECT_EQ( value, nullptr );
  EXPECT_THROW_WITH_MESSAGE( parser.parseInto( value, "{123 null}" ), "Syntax error at '123 null}'" );
  EXPECT_EQ( value, nullptr );
  EXPECT_THROW_WITH_MESSAGE( parser.parseInto( value, "123 null" ), "Early exit from loop at 'null'" );
  EXPECT_EQ( value, nullptr );
}
