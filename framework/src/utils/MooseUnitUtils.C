//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseUnitUtils.h"

#include <random>

namespace Moose::UnitUtils
{
TempFile::TempFile() : _path(generatePath()) {}

TempFile::~TempFile()
{
  std::error_code ec;
  std::filesystem::remove(path(), ec);
}

std::filesystem::path
TempFile::generatePath()
{
  static const std::string chars = "abcdefghijklmnopqrstuvwxyz0123456789";
  static thread_local std::mt19937 generator{std::random_device{}()};
  std::uniform_int_distribution<std::size_t> distribution(0, chars.size() - 1);
  std::string result;
  const std::size_t len = 10;
  result.reserve(len);
  for (std::size_t i = 0; i < len; ++i)
    result += chars[distribution(generator)];
  return std::filesystem::temp_directory_path() / std::filesystem::path("mooseunit." + result);
}

}
