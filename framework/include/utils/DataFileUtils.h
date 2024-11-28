//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>
#include <optional>

namespace Moose::DataFileUtils
{

/**
 * Context for where a data file came from
 */
enum class Context
{
  // Relative to the base (typically an input file)
  RELATIVE,
  // An absolute path
  ABSOLUTE,
  // From installed/in-tree data
  DATA,
  /// Unused for default
  INVALID
};

/**
 * Representation of a data file path
 */
struct Path
{
  Path() : path(), context(Context::INVALID), data_name() {}
  Path(const std::string & path,
       const Context context,
       const std::optional<std::string> & data_name = std::optional<std::string>())
    : path(path), context(context), data_name(data_name)
  {
  }
  // Path to the file
  std::string path;
  /// Context for the file (where it came from)
  Context context;
  /// The name of the data registry the file came from (with context == DATA)
  std::optional<std::string> data_name;
};

/**
 * Get the data path for a given path, searching the registered data
 *
 * @param path - The path; can be prefixed with <name>: to search only data from <name>
 * @param base - The base by which to search for the file relative to (optional)
 */
Path getPath(std::string path,
             const std::optional<std::string> & base = std::optional<std::string>());

/**
 * Get the data path for a given path, searching the registered data given an explicit
 * data search path.
 *
 * This exists primarily so that you don't need to call getPath("moose:file").
 *
 * @param data_name - The registered data name
 * @param path - The path
 * @param base - The base by which to search for the file relative to (optional)
 */
Path getPathExplicit(const std::string & data_name,
                     const std::string & path,
                     const std::optional<std::string> & base = std::optional<std::string>());
}
