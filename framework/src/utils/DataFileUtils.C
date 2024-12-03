//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DataFileUtils.h"
#include "Moose.h"
#include "MooseError.h"
#include "MooseUtils.h"
#include "Registry.h"

#include <filesystem>
#include <regex>

namespace Moose::DataFileUtils
{
Moose::DataFileUtils::Path
getPath(std::string path, const std::optional<std::string> & base)
{
  const auto & data_paths = Registry::getRegistry().getDataFilePaths();

  // Search for "<name>:" prefix which is a data name to limit the search to
  std::optional<std::string> data_name;
  std::smatch match;
  if (std::regex_search(path, match, std::regex("(?:(\\w+):)?(.*)")))
  {
    if (match[1].matched)
    {
      data_name = match[1];
      if (!data_paths.count(*data_name))
        mooseError("Data from '", *data_name, "' is not registered to be searched");
    }
    path = match[2];
  }
  else
    mooseError("Failed to parse path '", path, "'");

  const std::filesystem::path value_path = std::filesystem::path(path);

  // File is absolute, no need to search
  if (std::filesystem::path(path).is_absolute())
  {
    if (data_name)
      mooseError("Should not specify an absolute path along with a data name to search (requested "
                 "to search in '",
                 *data_name,
                 "')");
    if (MooseUtils::checkFileReadable(path, false, false, false))
      return {MooseUtils::canonicalPath(path), Context::ABSOLUTE};
    mooseError("The absolute path '", path, "' does not exist or is not readable.");
  }

  // Keep track of what was was searched for error context
  std::map<std::string, std::string> not_found;

  // Relative to the base, if provided
  if (base)
  {
    const auto relative_to_base = MooseUtils::pathjoin(*base, path);
    if (MooseUtils::checkFileReadable(relative_to_base, false, false, false))
      return {MooseUtils::canonicalPath(relative_to_base), Context::RELATIVE};
    not_found.emplace("working directory", MooseUtils::canonicalPath(*base));
  }

  // See if we should skip searching data
  std::optional<std::string> skip_data_reason;
  // Path starts with ./ so don't search data
  if (path.size() > 1 && path.substr(0, 2) == "./")
  {
    skip_data_reason = "begins with './'";
  }
  else
  {
    // Path resolves outside of . so don't search data
    const std::string proximate = std::filesystem::proximate(path).c_str();
    if (proximate.size() > 1 && proximate.substr(0, 2) == "..")
    {
      skip_data_reason = "resolves behind '.'";
    }
  }

  // Search data if we don't have a reason not to
  std::map<std::string, std::string> found;
  if (!skip_data_reason)
    for (const auto & [name, data_path] : data_paths)
    {
      // Explicit search, name doesn't match requested name
      if (data_name && name != *data_name) // explicit search
        continue;
      const auto file_path = MooseUtils::pathjoin(data_path, path);
      if (MooseUtils::checkFileReadable(file_path, false, false, false))
        found.emplace(name, MooseUtils::canonicalPath(file_path));
      else
        not_found.emplace(name + " data", data_path);
    }

  // Found exactly one
  if (found.size() == 1)
  {
    const auto & [name, data_path] = *found.begin();
    return {MooseUtils::canonicalPath(data_path), Context::DATA, name};
  }

  std::stringstream oss;
  // Found multiple
  if (found.size() > 1)
  {
    oss << "Multiple files were found when searching for the data file '" << path << "':\n\n";
    for (const auto & [name, data_path] : found)
      oss << "  " << name << ": " << data_path << "\n";
    const auto & first_name = found.begin()->first;
    oss << "\nYou can resolve this ambiguity by appending a prefix with the desired data name, for "
           "example:\n\n  "
        << first_name << ":" << path;
  }
  // Found none
  else
  {
    oss << "Unable to find the data file '" << path << "' anywhere.\n";
    if (not_found.size())
    {
      oss << "\nPaths searched:\n";
      for (const auto & [name, data_path] : not_found)
        oss << "  " << name << ": " << data_path << "\n";
    }
    if (skip_data_reason)
      oss << "\nData path(s) were not searched because search path " << *skip_data_reason << ".\n";
  }

  mooseError(oss.str());
}

Moose::DataFileUtils::Path
getPathExplicit(const std::string & data_name,
                const std::string & path,
                const std::optional<std::string> & base)
{
  return getPath(data_name + ":" + path, base);
}
}
