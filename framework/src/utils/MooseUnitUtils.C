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

namespace
{
std::filesystem::path
generatePath()
{
  static const std::string chars = "abcdefghijklmnopqrstuvwxyz0123456789";
  static thread_local std::mt19937 generator{std::random_device{}()};
  std::uniform_int_distribution<std::size_t> distribution(0, chars.size() - 1);
  std::string result;
  // Ten base-36 characters provide more than 3e15 possible paths.
  constexpr std::size_t random_name_length = 10;
  result.reserve(random_name_length);
  while (result.size() < random_name_length)
    result += chars[distribution(generator)];
  return std::filesystem::temp_directory_path() / std::filesystem::path("mooseunit." + result);
}
}

namespace Moose::UnitUtils
{
TempFile::TempFile() : _path(generatePath()) {}

TempFile::~TempFile()
{
  std::error_code ec;
  std::filesystem::remove(path(), ec);
}

ScopedTestDirectory::ScopedTestDirectory(const std::vector<std::filesystem::path> & inputs)
  : _original_path(std::filesystem::current_path()), _path(createPath())
{
  try
  {
    for (const auto & input : inputs)
      std::filesystem::copy(
          input, _path / input.filename(), std::filesystem::copy_options::recursive);
    std::filesystem::current_path(_path);
  }
  catch (...)
  {
    std::error_code ec;
    std::filesystem::remove_all(_path, ec);
    throw;
  }
}

ScopedTestDirectory::~ScopedTestDirectory()
{
  std::error_code ec;
  std::filesystem::current_path(_original_path, ec);
  if (!ec)
    std::filesystem::remove_all(_path, ec);
}

std::filesystem::path
ScopedTestDirectory::createPath()
{
  while (true)
  {
    const auto path = generatePath();
    if (std::filesystem::create_directory(path))
      return path;
  }
}

}
