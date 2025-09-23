//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "InputParameters.h"

#define usingFunctionInterfaceMembers                                                              \
  using FunctionInterface::getFunction;                                                            \
  using FunctionInterface::getFunctionByName

// Forward declarations
class Function;
class FEProblemBase;
class Function;
class InputParameters;
class MooseObject;

template <typename T>
InputParameters validParams();

/**
 * Interface for objects that need to use functions
 *
 * Inherit from this class at a very low level to make the getFunction method
 * available.
 */
class FunctionInterface
{
public:
  FunctionInterface(const MooseObject * moose_object);

#ifdef MOOSE_KOKKOS_ENABLED
  /**
   * Special constructor used for Kokkos functor copy during parallel dispatch
   */
  FunctionInterface(const FunctionInterface & object, Moose::Kokkos::FunctorCopy);
#endif

  static InputParameters validParams();

  /**
   * Get a function with a given name
   * @param name The name of the parameter key of the function to retrieve
   * @return The function with name associated with the parameter 'name'
   */
  const Function & getFunction(const std::string & name) const;

  /**
   * Get a function with a given name
   * @param name The name of the function to retrieve
   * @return The function with name 'name'
   */
  const Function & getFunctionByName(const FunctionName & name) const;

  /**
   * Determine if the function exists
   *
   * @param param_name The name of the function parameter
   * @param index The index of the function
   * @return True if the function exists
   */
  bool hasFunction(const std::string & param_name) const;

  /**
   * Determine if the function exists
   *
   * @param name The name of the function
   * @return True if the function exists
   */
  bool hasFunctionByName(const FunctionName & name) const;

private:
  /// Parameters of the object with this interface
  const InputParameters & _fni_params;

  /// Reference to FEProblemBase instance
  FEProblemBase & _fni_feproblem;

  /// Thread ID
  const THREAD_ID _fni_tid;
};
