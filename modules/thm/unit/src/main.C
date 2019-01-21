#include "THMApp.h"
#include "gtest/gtest.h"

#include "Moose.h"
#include "MooseInit.h"
#include "AppFactory.h"

#include <fstream>
#include <string>

PerfLog Moose::perf_log("thm_unit");

GTEST_API_ int
main(int argc, char ** argv)
{
  // gtest removes (only) its args from argc and argv - so this  must be before moose init
  testing::InitGoogleTest(&argc, argv);

  MooseInit init(argc, argv);
  registerApp(THMApp);
  Moose::_throw_on_error = true;

  return RUN_ALL_TESTS();
}
