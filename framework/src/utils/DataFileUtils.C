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

namespace Moose::DataFileUtils
{
Moose::DataFileUtils::Path
getPath(const std::string & path,
        const std::optional<std::string> & base,
        const std::optional<std::string> & data_name)
{
  const std::filesystem::path value_path = std::filesystem::path(std::string(path));

  // File is absolute, no need to search
  if (std::filesystem::path(path).is_absolute())
  {
    if (MooseUtils::checkFileReadable(path, false, false, false))
      return {MooseUtils::absolutePath(path), Context::ABSOLUTE};
    mooseError("The absolute path '", path, "' does not exist or is not readable.");
  }

  // Relative to the base, if provided
  if (base)
  {
    const auto relative_to_base = MooseUtils::pathjoin(*base, path);
    if (MooseUtils::checkFileReadable(relative_to_base, false, false, false))
      return {MooseUtils::absolutePath(relative_to_base), Context::RELATIVE};
  }

  // Keep track of what was found and what is not
  std::map<std::string, std::string> not_found, found;

  // Search each registered data path for the relative path
  for (const auto & [name, data_path] : Registry::getRegistry().getDataFilePaths())
  {
    if (data_name && name != *data_name)
      continue;
    const auto file_path = MooseUtils::pathjoin(data_path, path);
    if (MooseUtils::checkFileReadable(file_path, false, false, false))
      found.emplace(name, MooseUtils::absolutePath(file_path));
    else
      not_found.emplace(name, data_path);
  }

  // Found exactly one
  if (found.size() == 1)
  {
    const auto & [name, data_path] = *found.begin();
    return {MooseUtils::absolutePath(data_path), Context::DATA, name};
  }

  std::stringstream oss;
  // Found multiple
  // TODO: Eventually, we could support a special syntax here that will allow a user
  // to specify where to get data from to resolve ambiguity. For example, something like
  // solid_mechancs:path/to/data
  if (found.size() > 1)
  {
    oss << "Multiple files were found when searching for the data file '" << path << "':\n\n";
    for (const auto & [name, data_path] : found)
      oss << "  " << name << ": " << data_path << "\n";
  }
  // Found none
  else
  {
    oss << "Unable to find the data file '" << path << "' anywhere.\n\n";
    if (not_found.size())
    {
      oss << "Paths searched:\n";
      for (const auto & [name, data_path] : not_found)
        oss << "  " << name << ": " << data_path << "\n";
    }
  }

  mooseError(oss.str());
}
}
