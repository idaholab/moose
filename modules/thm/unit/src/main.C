#include "gtest/gtest.h"
#include "AppFactory.h"
#include "MooseInit.h"
#include "THMTestApp.h"

PerfLog Moose::perf_log("THM unit tests");

GTEST_API_ int
main(int argc, char ** argv)
{
  // gtest removes (only) its args from argc and argv - so this  must be before moose init
  testing::InitGoogleTest(&argc, argv);

  MooseInit init(argc, argv);
  registerApp(THMTestApp);
  Moose::_throw_on_error = true;

  return RUN_ALL_TESTS();
}
