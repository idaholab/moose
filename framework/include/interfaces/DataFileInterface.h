//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include <string>

class ParallelParamObject;

/**
 * Interface for objects that need to resolve data file paths (MooseObject and Action)
 */
class DataFileInterface
{
public:
  /**
   * The parameter type this interface expects for a data file name.
   */
  using DataFileParameterType = DataFileName;

  /**
   * Constructing the object
   * @param parent Parent object (either MooseObject or Action) for params and  output
   */
  DataFileInterface(const ParallelParamObject & parent);

  /**
   * Deprecated method.
   *
   * The data file paths are now automatically set within the InputParameters
   * object, so using getParam<DataFileName>("param_name") is now sufficient.
   */
  std::string getDataFileName(const std::string & param) const;

  /**
   * Deprecated method.
   *
   * Use getDataFilePath() instead.
   */
  std::string getDataFileNameByName(const std::string & relative_path) const;

  /**
   * Returns the path of a data file for a given relative file path.
   * This can be used for hardcoded datafile names and will search the same locations
   * as getDataFileName
   */
  std::string getDataFilePath(const std::string & relative_path) const;

private:
  const ParallelParamObject & _parent;
};
