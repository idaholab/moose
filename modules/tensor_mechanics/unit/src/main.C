//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <fstream>
#include <string>

#include "Moose.h"
#include "MooseInit.h"
#include "AppFactory.h"

#include "gtest/gtest.h"

#include "TensorMechanicsApp.h"

PerfLog Moose::perf_log("gtest");

GTEST_API_ int
main(int argc, char ** argv)
{
  // gtest removes (only) its args from argc and argv - so this must be before moose init
  testing::InitGoogleTest(&argc, argv);

  MooseInit init(argc, argv);
  registerApp(TensorMechanicsApp);
  Moose::_throw_on_error = true;
  Moose::_throw_on_warning = true;

  return RUN_ALL_TESTS();
}
