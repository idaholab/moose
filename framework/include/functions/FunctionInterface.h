//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTIONINTERFACE_H
#define FUNCTIONINTERFACE_H

#include "MooseTypes.h"

#define usingFunctionInterfaceMembers                                                              \
  using FunctionInterface::getFunction;                                                            \
  using FunctionInterface::getFunctionByName

// Forward declarations
class Function;
class FEProblemBase;
class FunctionInterface;
class Function;
class InputParameters;
class MooseObject;

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<FunctionInterface>();

/**
 * Interface for objects that need to use functions
 *
 * Inherit from this class at a very low level to make the getFunction method
 * available.
 */
class FunctionInterface
{
public:
  /**
   * @param params The parameters used by the object being instantiated. This
   *        class needs them so it can get the function named in the input file,
   *        but the object calling getFunction only needs to use the name on the
   *        left hand side of the statement "function = func_name"
   */
  FunctionInterface(const MooseObject * moose_object);

  /**
   * Get a function with a given name
   * @param name The name of the parameter key of the function to retrieve
   * @return The function with name associated with the parameter 'name'
   */
  Function & getFunction(const std::string & name);

  /**
   * Get a function with a given name
   * @param name The name of the function to retrieve
   * @return The function with name 'name'
   */
  Function & getFunctionByName(const FunctionName & name);

private:
  /// Parameters of the object with this interface
  const InputParameters & _fni_params;

  /// Reference to FEProblemBase instance
  FEProblemBase & _fni_feproblem;

  /// Thread ID
  THREAD_ID _fni_tid;
};

#endif // FUNCTIONINTERFACE_H
