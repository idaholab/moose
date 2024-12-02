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

#include <map>
#include <string>

class RegistryTest : public ::testing::Test
{
public:
  virtual void SetUp() override;
  virtual void TearDown() override;

  std::map<std::string, std::string> _old_data_file_paths;
  std::map<std::string, std::string> _old_repos;
};
