//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
getPath(std::string path, const GetPathOptions & options)
{
  const auto & data_paths = Registry::getRegistry().getDataFilePaths();

  // Search for "<name>:" prefix which is an explicit data name to search
  std::optional<std::pair<std::string, std::string>> explicit_data;
  if (std::smatch match; std::regex_search(path, match, std::regex("(?:([a-z0-9_]+):)?(.*)")))
  {
    // Has an explicit data name
    if (match[1].matched)
    {
      if (const auto it = data_paths.find(match[1].str()); it != data_paths.end())
        explicit_data = *it;
      else
        mooseError("Data from '", match[1], "' is not registered to be searched");
    }
    path = match[2];
  }
  else
    mooseError("Failed to parse path '", path, "'");

  const std::filesystem::path value_path = std::filesystem::path(path);

  // Helper for erroring if the data name is explicitly set
  const auto if_data_name_error = [&explicit_data](const auto & message)
  {
    if (explicit_data)
      mooseError("Cannot use ",
                 message,
                 " along with a data name to search (requested "
                 "to search in '",
                 explicit_data->first,
                 "')");
  };

  // File is absolute, no need to search
  if (std::filesystem::path(path).is_absolute())
  {
    // Shouldn't provide an absolute path and an explicit data name
    if_data_name_error("an absolute path");

    // File exists, we're good to go
    if (MooseUtils::checkFileReadable(path, false, false, false))
      return {MooseUtils::canonicalPath(path), Context::ABSOLUTE};

    // If absolute and graceful, return the bad absolute path
    if (options.graceful)
      return {MooseUtils::canonicalPath(path), Context::ABSOLUTE_NOT_FOUND};

    // If absolute and not graceful, nothing else to do
    mooseError("The absolute path '", path, "' does not exist or is not readable.");
  }

  // Keep track of what was was searched for error context
  std::map<std::string, std::string> not_found;

  // Because no explicit data name is set, we always want to prefer
  // checking relative paths first before searching the data
  if (!explicit_data)
  {
    const std::string base = options.base ? *options.base : std::filesystem::current_path().c_str();
    const auto relative_to_base = MooseUtils::pathjoin(base, path);

    // Relative path exists
    if (MooseUtils::checkFileReadable(relative_to_base, false, false, false))
      return {MooseUtils::canonicalPath(relative_to_base), Context::RELATIVE};

    // Without an explicit data name set, if we're set to not search all data
    // there's nothing else to check here so we'll avoid the error at the bottom
    // and exit immediately
    if (!options.search_all_data && options.graceful)
      return {MooseUtils::canonicalPath(relative_to_base), Context::RELATIVE_NOT_FOUND};

    // Mark that we searched the working directory
    not_found.emplace("working directory", MooseUtils::canonicalPath(base));
  }

  // See if we should skip searching data
  std::optional<std::string> skip_data_reason;
  // Path starts with ./ so don't search data
  if (path.size() > 1 && path.substr(0, 2) == "./")
  {
    // Shouldn't provide a relative path and an explicit data name
    if_data_name_error("a path that starts with './'");

    // Mark that we're not checking data because it is a relative path
    skip_data_reason = "begins with './'";
  }
  // Path resolves outside of . so don't search data
  else if (const std::string proximate = std::filesystem::proximate(path).c_str();
           (proximate.size() > 1 && proximate.substr(0, 2) == ".."))
  {
    // Shouldn't provide a relative path and an explicit data name
    if_data_name_error("a relative path");

    // Mark that we're not checking data because it's out of the cwd
    skip_data_reason = "resolves behind '.'";
  }

  // Search the data if we found a reason not to and it's set
  // to search data or we have an explicit data name to search
  std::map<std::string, std::string> found;
  if (!skip_data_reason && (options.search_all_data || explicit_data))
  {
    const auto check_data_path = [&found, &not_found, &path](const auto & entry)
    {
      const auto & [name, data_path] = entry;
      const auto file_path = MooseUtils::pathjoin(data_path, path);
      if (MooseUtils::checkFileReadable(file_path, false, false, false))
        found.emplace(name, MooseUtils::canonicalPath(file_path));
      else
        not_found.emplace(name + " data", data_path);
    };

    // Explicit search, just searching one
    if (explicit_data)
    {
      check_data_path(*explicit_data);
      if (found.empty())
        mooseError("The path '", path, "' was not found in data from '", explicit_data->first, "'");
    }
    // Search all data paths
    else
    {
      for (const auto & entry : data_paths)
        check_data_path(entry);
    }
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
    oss << "Unable to find the data file '" << path << "'.\n";
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
  GetPathOptions options;
  options.base = base;
  return getPath(data_name + ":" + path, options);
}
}
