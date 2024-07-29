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
  const std::string relative_to_context = std::filesystem::absolute(base / value_path).c_str();
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
  /// - relative to the running binary (assuming the application is installed)
  const auto share_dir = MooseUtils::pathjoin(Moose::getExecutablePath(), "..", "share");
  if (MooseUtils::pathIsDirectory(share_dir))
  {
    const auto dirs = MooseUtils::listDir(share_dir, false);
    for (const auto & data_dir : dirs)
    {
      const auto path = MooseUtils::pathjoin(data_dir, "data", relative_path);
      if (MooseUtils::checkFileReadable(path, false, false, false))
      {
        if (param)
          _parent.paramInfo(
              *param, "Data file '", path, "' found in an installed app distribution.");
        else
          mooseInfo("Data file '", path, "' found in an installed app distribution.");
        return path;
      }
    }
  }

  /// - relative to all registered data file directories
  for (const auto & data_dir : Registry::getRegistry().getDataFilePaths())
  {
    const auto path = MooseUtils::pathjoin(data_dir, relative_path);
    if (MooseUtils::checkFileReadable(path, false, false, false))
    {
      if (param)
        _parent.paramInfo(*param, "Data file '", path, "' found in a source repository.");
      else
        mooseInfo("Data file '", path, "' found in a source repository.");
      return path;
    }
  }

  mooseException(param ? _parent.parameters().inputLocation(*param) : _parent.name(),
                 ": Unable to find data file '",
                 relative_path,
                 "' anywhere");
}
