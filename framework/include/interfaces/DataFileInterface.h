//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>

/**
 * Interface for objects that need to resolve data file paths (MooseObject and Action)
 */
template <class T>
class DataFileInterface
{
public:
  /**
   * Constructing the object
   * @param parent Parent object (either MooseObject or Action) for params and  output
   */
  DataFileInterface(const T & parent);

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
   * as getDataFileName. The optional param pointer can be used to turn the mooseErrors this
   * function emits into paramErrors
   */
  std::string getDataFileNameByName(const std::string & name,
                                    const std::string * param = nullptr) const;

private:
  const T & _parent;
};
