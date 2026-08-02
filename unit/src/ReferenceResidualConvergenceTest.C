//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "Moose.h"
#include "MooseMain.h"
#include "XTermConstants.h"

namespace
{
struct Args
{
  Args(const std::vector<std::string> & args) : _args(args)
  {
    _args.insert(_args.begin(), "/path/to/exe");
    for (auto & arg : _args)
      _argv.push_back((char *)arg.data());
    _argv.push_back(nullptr);
  }

  int argc() const { return _argv.size() - 1; }
  char ** argv() { return _argv.data(); }

  std::vector<std::string> _args;
  std::vector<char *> _argv;
};
}

TEST(ReferenceResidualConvergenceTest, quantityColors)
{
  const bool color_was_enabled = Moose::colorConsole();

  testing::internal::CaptureStdout();
  Args args({"-i", "files/ReferenceResidualConvergenceTest/color.i", "--color", "on"});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  app->run();
  Moose::out << std::flush;
  const auto output = testing::internal::GetCapturedStdout();

  Moose::setColorConsole(color_was_enabled, color_was_enabled);

  bool found_colored_row = false;
  std::size_t row_begin = 0;
  while ((row_begin = output.find("u-> res: ", row_begin)) != std::string::npos)
  {
    const auto row_end = output.find('\n', row_begin);
    const auto row = output.substr(row_begin, row_end - row_begin);
    const auto residual_color = row.find(std::string("res: ") + XTERM_YELLOW);
    const auto reference_label = row.find("  ref: ");
    const auto residual_reset = row.find(XTERM_DEFAULT, residual_color);
    const auto ratio_color = row.find(std::string("res/ref: ") + XTERM_GREEN);
    const auto ratio_reset = row.find(XTERM_DEFAULT, ratio_color);

    if (residual_color != std::string::npos && reference_label != std::string::npos &&
        residual_reset < reference_label && ratio_color != std::string::npos &&
        ratio_reset != std::string::npos)
    {
      found_colored_row = true;
      break;
    }

    if (row_end == std::string::npos)
      break;
    row_begin = row_end + 1;
  }

  EXPECT_TRUE(found_colored_row)
      << "Expected a reference residual row with a yellow residual and green ratio:\n"
      << output;
}
