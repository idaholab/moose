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
#include "MooseObject.h"
#include "Action.h"

template <class T>
DataFileInterface<T>::DataFileInterface(const T & parent) : _parent(parent)
{
}

template <class T>
std::string
DataFileInterface<T>::getDataFileName(const std::string & param) const
{
  /// - relative to the input file directory
  {
    const auto & absolute_path = _parent.template getParam<FileName>(param);
    if (MooseUtils::checkFileReadable(absolute_path, false, false, false))
    {
      _parent.paramInfo(param, "Data file '", absolute_path, "' found relative to the input file.");
      return absolute_path;
    }
  }

  const auto & relative_path = _parent.parameters().rawParamVal(param);
  return getDataFileNameByName(relative_path, &param);
}

template <class T>
std::string
DataFileInterface<T>::getDataFileNameByName(const std::string & relative_path,
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

template class DataFileInterface<Action>;
template class DataFileInterface<MooseObject>;
