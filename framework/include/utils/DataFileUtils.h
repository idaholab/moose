//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  /// Relative to the base (typically an input file)
  RELATIVE,
  /// An absolute path
  ABSOLUTE,
  /// From installed/in-tree data
  DATA,
  /// Relative to the base, but not found
  RELATIVE_NOT_FOUND,
  /// Absolute, but not found
  ABSOLUTE_NOT_FOUND,
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
 * Options to be passed to getPath().
 */
struct GetPathOptions
{
  /// The base path by which to search for relative paths. This is usually
  /// the folder that an input file is in so that paths are searched relative
  /// to where that input file is.
  std::optional<std::string> base;
  /// Whether or not to search all registered data.
  bool search_all_data = true;
  /// Whether or not to error whenever a path is not found. If this is true,
  /// the only time an error is emitted is when an explicit data name is
  /// specified and not registered or when an explicit data name is specified
  /// and the file was not found within that registered data folder.
  bool graceful = false;
};

/**
 * Get the data path for a given path, searching the registered data
 *
 * @param path The path; can be prefixed with <name>: to search only data from <name>
 * @param options Search options; see docstring for GetPathOptions for more info
 */
Path getPath(std::string path, const GetPathOptions & options = {});

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
