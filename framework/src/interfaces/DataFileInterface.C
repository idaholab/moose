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
#include "DataFileUtils.h"
#include "MooseUtils.h"

#include <optional>

DataFileInterface::DataFileInterface(const ParallelParamObject & parent) : _parent(parent) {}

std::string
DataFileInterface::getDataFileName(const std::string & param) const
{
  _parent.mooseDeprecated("getDataFileName() is deprecated. The file path is now directly set "
                          "within the InputParameters.\nUse getParam<DataFileName>(\"",
                          param,
                          "\") instead.");
  return _parent.getParam<DataFileName>(param);
}

std::string
DataFileInterface::getDataFileNameByName(const std::string & relative_path) const
{
  _parent.mooseDeprecated("getDataFileNameByName() is deprecated. Use getDataFilePath(\"",
                          relative_path,
                          "\") instead.");
  return getDataFilePath(relative_path);
}

std::string
DataFileInterface::getDataFilePath(const std::string & relative_path) const
{
  // This should only ever be used with relative paths. There is no point to
  // use this search path with an absolute path.
  if (std::filesystem::path(relative_path).is_absolute())
    _parent.mooseWarning("While using getDataFilePath(\"",
                         relative_path,
                         "\"): This API should not be used for absolute paths.");

  // Throw on error so that if getPath() fails, we can throw an error
  // with the context of _parent.mooseError()
  const auto throw_on_error_before = Moose::_throw_on_error;
  Moose::_throw_on_error = true;
  std::optional<std::string> error;

  // This will search the data paths for this relative path
  Moose::DataFileUtils::Path found_path;
  try
  {
    found_path = Moose::DataFileUtils::getPath(relative_path);
  }
  catch (std::exception & e)
  {
    error = e.what();
  }

  Moose::_throw_on_error = throw_on_error_before;
  if (error)
    _parent.mooseError(*error);

  mooseAssert(found_path.context == Moose::DataFileUtils::Context::DATA,
              "Should only ever obtain data");
  mooseAssert(found_path.data_name, "Should be set");

  const std::string msg =
      "Using data file '" + found_path.path + "' from " + *found_path.data_name + " data";
  _parent.mooseInfo(msg);

  return found_path.path;
}
