//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DataFileInterface.h"
#include "MooseError.h"
#include "ParallelParamObject.h"
#include "Registry.h"

#include <filesystem>

DataFileInterface::DataFileInterface(const ParallelParamObject & parent) : _parent(parent) {}

std::string
DataFileInterface::getDataFileName(const std::string & param) const
{
  // The path from the parameters, which has not been modified because it is a DataFileName
  const auto & value = _parent.template getParam<DataFileParameterType>(param);
  if (value.empty())
    _parent.paramInfo(param, "Data file name is empty");

  const std::filesystem::path value_path = std::filesystem::path(std::string(value));

  // If the file is absolute, we should reference that directly and don't need to add
  // any info beacuse this is not ambiguous
  if (value_path.is_absolute() && MooseUtils::checkFileReadable(value, false, false, false))
    return value;

  // Look relative to the input file
  const auto base = _parent.parameters().getParamFileBase(param);
  const std::string relative_to_context =
      std::filesystem::weakly_canonical(base / value_path).c_str();
  if (MooseUtils::checkFileReadable(relative_to_context, false, false, false))
  {
    _parent.paramInfo(param, "Data file '", value, "' found relative to the input file.");
    return relative_to_context;
  }

  // Isn't absolute and couldn't find relative to the input file, so search the data
  return getDataFileNameByName(value, &param);
}

std::string
DataFileInterface::getDataFileNameByName(const std::string & relative_path,
                                         const std::string * param) const
{
  std::map<std::string, std::string> not_found, found;

  // Search each registered data path for the relative path
  for (const auto & [name, path] : Registry::getRegistry().getDataFilePaths())
  {
    const auto file_path = MooseUtils::pathjoin(path, relative_path);
    const std::string abs_file_path = std::filesystem::weakly_canonical(file_path).c_str();
    if (MooseUtils::checkFileReadable(abs_file_path, false, false, false))
      found.emplace(name, abs_file_path);
    else
      not_found.emplace(name, path);
  }

  // Found exactly one
  if (found.size() == 1)
  {
    const auto & [name, path] = *(found.begin());
    if (param)
      _parent.paramInfo(*param, "Using data file '", path, "' from ", name);
    else
      _parent.mooseInfo("Using data file '", path, "' from ", name);
    return path;
  }

  std::stringstream oss;
  // Found multiple
  if (found.size() > 1)
  {
    oss << "Multiple files were found when searching for the data file '" << relative_path
        << "':\n\n";
    for (const auto & [name, path] : found)
      oss << "  " << name << ": " << path << "\n";
  }
  // Found none
  else
  {
    oss << "Unable to find the data file '" << relative_path << "' anywhere.";
    if (not_found.size())
    {
      oss << " Paths searched:\n";
      for (const auto & [name, path] : not_found)
        oss << "  " << name << ": " << path << "\n";
    }
  }

  if (param)
    _parent.paramError(*param, oss.str());
  else
    _parent.mooseError(oss.str());
}
