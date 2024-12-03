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

#include "DataFileUtils.h"

#include <array>
#include <string>

class DataFileUtilsTest : public ::testing::Test
{
public:
  DataFileUtilsTest();

  virtual void SetUp() override;
  virtual void TearDown() override;

  void testData(const Moose::DataFileUtils::Path & path,
                const unsigned int index,
                const std::string & relative_path) const;
  void testRelative(const Moose::DataFileUtils::Path & path,
                    const std::string & relative_path) const;
  void testAbsolute(const Moose::DataFileUtils::Path & path,
                    const std::string & absolute_path) const;

  const std::array<std::string, 2> _names;
  const std::array<std::string, 2> _paths;
  const std::array<std::string, 2> _can_paths;
  const std::string _cwd;
  std::map<std::string, std::string> _old_data_file_paths;
};
