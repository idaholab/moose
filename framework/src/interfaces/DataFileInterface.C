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

DataFileInterface::DataFileInterface(const ParallelParamObject & parent) : _parent(parent) {}

std::string
DataFileInterface::getDataFileName(const std::string & param) const
{
  // The path from the parameters, which has not been modified because it is a DataFileName
  const auto & path = _parent.template getParam<DataFileParameterType>(param);
  if (path.empty())
    _parent.paramInfo(param, "Data file name is empty");

  return getDataFileNameInternal(path, &param);
}

std::string
DataFileInterface::getDataFileNameByPath(const std::string & path) const
{
  return getDataFileNameInternal(path, nullptr);
}

std::string
DataFileInterface::getDataFileNameInternal(const std::string & path,
                                           const std::string * param) const
{
  const std::string base =
      param ? _parent.parameters().getFileBase(*param) : _parent.parameters().getFileBase();

  Moose::DataFileUtils::Path found_path;
  try
  {
    found_path = Moose::DataFileUtils::getPath(path, base);
  }
  catch (MooseException & e)
  {
    if (param)
      _parent.paramError(*param, e.what());
    else
      _parent.mooseError(e.what());
  }

  if (found_path.context == Moose::DataFileUtils::Context::RELATIVE)
  {
    mooseAssert(!info.data_name, "Should not be set");
    mooseAssert(param, "Should only hit when param is set");
    _parent.paramInfo(*param, "Data file '", path, "' found relative to the input file.");
  }
  else if (found_path.context == Moose::DataFileUtils::Context::DATA)
  {
    mooseAssert(found_path.data_name, "Should be set");
    const std::string msg =
        "Using data file '" + found_path.path + "' from " + *found_path.data_name + " data";
    if (param)
      _parent.paramInfo(*param, msg);
    else
      _parent.mooseInfo(msg);
  }
  else
    mooseAssert(found_path.context == Moose::DataFileUtils::Context::ABSOLUTE, "Missing branch");

  return found_path.path;
}
