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
#include "MooseObject.h"
#include "InputParameters.h"

#define usingFunctionInterfaceMembers                                                              \
  using FunctionInterface::getFunction;                                                            \
  using FunctionInterface::getFunctionByName

// Forward declarations
class FEProblemBase;
class Function;
class InputParameters;

namespace Moose
{
class FunctionBase;
}

#ifdef MOOSE_KOKKOS_ENABLED
namespace Moose::Kokkos
{
class Function;
}
#endif

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
  FunctionInterface(const FunctionInterface & object, const Moose::Kokkos::FunctorCopy & key);
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
   * @param param_name The name of the function parameter
   * @param index The index of the function
   * @return True if the function exists
   */
  bool hasFunction(const std::string & param_name) const;

  /**
   * Determine if the function exists
   * @param name The name of the function
   * @return True if the function exists
   */
  bool hasFunctionByName(const FunctionName & name) const;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get a Kokkos function of an abstract type with a given name
   * Calling this function will error out currently if Kokkos was configured with GPU
   * @param name The name of the parameter key of the Kokkos function to retrieve
   * @return The copy of the Kokkos function of the abstract type with name associated with the
   * parameter 'name'
   */
  Moose::Kokkos::Function getKokkosFunction(const std::string & name) const;

  /**
   * Get a Kokkos function of an abstract type with a given name
   * Calling this function will error out currently if Kokkos was configured with GPU
   * @param name The name of the Kokkos function to retrieve
   * @return The copy of the Kokkos function of the abstract type with name 'name'
   */
  Moose::Kokkos::Function getKokkosFunctionByName(const FunctionName & name) const;

  /**
   * Get a Kokkos function of a concrete type with a given name
   * @tparam T The Kokkos function type
   * @param name The name of the parameter key of the Kokkos function to retrieve
   * @return The reference of the Kokkos function of the concrete type with name associated with the
   * parameter 'name'. Always store it in a reference wrapper if to be used on GPU.
   */
  template <typename T>
  const T & getKokkosFunction(const std::string & name) const;

  /**
   * Get a Kokkos function of a concrete type with a given name
   * @tparam T The Kokkos function type
   * @param name The name of the Kokkos function to retrieve
   * @return The reference of the Kokkos function of the concrete type with name 'name'. Always
   * store it in a reference wrapper if to be used on GPU.
   */
  template <typename T>
  const T & getKokkosFunctionByName(const FunctionName & name) const;

  /**
   * Determine if the Kokkos function exists
   * @param param_name The name of the Kokkos function parameter
   * @param index The index of the Kokkos function
   * @return True if the Kokkos function exists
   */
  bool hasKokkosFunction(const std::string & param_name) const;

  /**
   * Determine if the Kokkos function exists
   * @param name The name of the Kokkos function
   * @return True if the Kokkos function exists
   */
  bool hasKokkosFunctionByName(const FunctionName & name) const;
#endif

private:
#ifdef MOOSE_KOKKOS_ENABLED
  /// Helper function to retrieve a Kokkos function
  const Moose::FunctionBase * getKokkosFunctionByNameHelper(const FunctionName & name) const;

  /// Reference to the object
  const MooseObject & _fni_object;
#endif

  /// Parameters of the object with this interface
  const InputParameters & _fni_params;

  /// Reference to FEProblemBase instance
  FEProblemBase & _fni_feproblem;

  /// Thread ID
  const THREAD_ID _fni_tid;
};

#ifdef MOOSE_KOKKOS_SCOPE
template <typename T>
const T &
FunctionInterface::getKokkosFunction(const std::string & name) const
{
  return getKokkosFunctionByName<T>(_fni_params.get<FunctionName>(name));
}

template <typename T>
const T &
FunctionInterface::getKokkosFunctionByName(const FunctionName & name) const
{
  auto function = dynamic_cast<const T *>(getKokkosFunctionByNameHelper(name));

  if (!function)
    _fni_object.mooseError(
        "Kokkos function '", name, "' is not of type '", MooseUtils::prettyCppType<T>(), "'");

  return *function;
}
#endif
