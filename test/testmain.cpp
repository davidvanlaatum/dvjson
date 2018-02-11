#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <stdlib.h>       // for getenv

int main( int argc, char **argv ) {
  testing::InitGoogleMock( &argc, argv );

  int exit = RUN_ALL_TESTS();
  if ( getenv( "IGNORE_EXIT_CODE" ) != nullptr ) {
    return 0;
  }
  return exit;
}
