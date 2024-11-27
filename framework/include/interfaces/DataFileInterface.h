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
   * Returns the path of a data file for a given FileName type parameter, searching
   * (in the following order)
   * - relative to the input file directory
   * - relative to the running binary (assuming the application is installed)
   * - relative to all registered data file directories
   */
  std::string getDataFileName(const std::string & param) const;

  /**
   * Returns the path of a data file for a given relative file path.
   * This can be used for hardcoded datafile names and will search the same locations
   * as getDataFileName
   */
  std::string getDataFileNameByPath(const std::string & path) const;

private:
  /**
   * Internal helper for getting a data file name.
   */
  std::string getDataFileNameInternal(const std::string & path, const std::string * param) const;

  const ParallelParamObject & _parent;
};
