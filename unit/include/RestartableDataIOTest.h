//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest_include.h"

#include <memory>
#include <map>

class MooseApp;

class RestartableDataIOTest : public ::testing::Test
{
public:
  struct DataInfo
  {
    unsigned int value;
    bool no_declare;
  };

protected:
  void SetUp() override;

  std::shared_ptr<MooseApp> _app;
};
