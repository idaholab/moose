//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseVariableFE.h"
#include "NonlinearSystemBase.h"
#include "Enumerate.h"

template <class T>
class JvarMapInterfaceBase;

/**
 * Interface class ("Veneer") for Kernel to provide a mapping from 'jvar' in
 * computeQpOffDiagJacobian into the _coupled_moose_vars array.
 *
 * This class is useful in conjunction with DerivativeMaterialInterface,
 * where vectors of material property derivatives with respect to all coupled
 * variables (iterating over _coupled_moose_vars array) are generated.
 * The mapping enabled the look up of the correct material property derivatives
 * for the current jvar.
 */
template <class T>
class JvarMapKernelInterface : public JvarMapInterfaceBase<T>
{
public:
  JvarMapKernelInterface(const InputParameters & parameters);
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
};

/**
 * Interface class ("Veneer") for IntegratedBC to provide a mapping from 'jvar' in
 * computeJacobianBlock into the _coupled_moose_vars array.
 *
 * This class is useful in conjunction with DerivativeMaterialInterface,
 * where vectors of material property derivatives with respect to all coupled
 * variables (iterating over _coupled_moose_vars array) are generated.
 * The mapping enabled the look up of the correct material property derivatives
 * for the current jvar.
 */
template <class T>
class JvarMapIntegratedBCInterface : public JvarMapInterfaceBase<T>
{
public:
  JvarMapIntegratedBCInterface(const InputParameters & parameters);
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
};

/**
 * Base class ("Veneer") that implements the actual mapping from 'jvar' in
 * into the _coupled_moose_vars array.
 */
template <class T>
class JvarMapInterfaceBase : public T
{
public:
  typedef std::vector<int> JvarMap;

  static InputParameters validParams();

  JvarMapInterfaceBase(const InputParameters & parameters);

  /// Return index into the _coupled_moose_vars array for a given jvar
  unsigned int mapJvarToCvar(unsigned int jvar);

  /**
   * Return an index into a specific coupled variable vector for a given jvar. A
   * negative return value indicates that the jvar value does not point to a
   * variable in the couple variable vector corresponding to the mapped parameter.
   */
  int mapJvarToCvar(unsigned int jvar, const JvarMap & jvar_map);

  /// Obtain the map connecting libmesh variable ID number to its position in the _coupled_moose_vars vector
  const JvarMap & getJvarMap() { return _jvar_map; }

  /// Make a specific map for a given parameter name representing a couple variable (vector)
  const JvarMap & getParameterJvarMap(std::string parameter_name);

  /**
   * Set the cvar value to the mapped jvar value and return true if the mapping exists.
   * Otherwise return false.
   *
   * @param[in] jvar Variable number passed as argument to computeQpOffDiagJacobian
   * @param[out] cvar Corresponding index into the _coupled_moose_vars array
   * @return true if the requested variable is coupled, false otherwise
   */
  bool mapJvarToCvar(unsigned int jvar, unsigned int & cvar);

protected:
  /// number of coupled moose variables
  const unsigned int _n_args;

private:
  /// number of nonlinear variables in the system
  const std::size_t _jvar_max_size;

  /// look-up table to determine the _coupled_moose_vars index for the jvar parameter
  JvarMap _jvar_map;

  /// map of local look-up tables for specific parameters
  std::map<std::string, JvarMap> _jvar_local_map;

  friend class JvarMapKernelInterface<T>;
  friend class JvarMapIntegratedBCInterface<T>;
};

template <class T>
InputParameters
JvarMapInterfaceBase<T>::validParams()
{
  auto params = T::validParams();
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  params.deprecateCoupledVar("args", "coupled_variables", "02/07/2024");

  return params;
}

template <class T>
JvarMapInterfaceBase<T>::JvarMapInterfaceBase(const InputParameters & parameters)
  : T(parameters),
    _n_args(this->_coupled_standard_moose_vars.size()),
    _jvar_max_size(this->_fe_problem.getNonlinearSystemBase().nVariables()),
    _jvar_map(_jvar_max_size, -1)
{
  // populate map
  for (auto it : Moose::enumerate(this->_coupled_moose_vars))
  {
    auto number = it.value()->number();

    // skip AuxVars as off-diagonal jacobian entries are not calculated for them
    if (number < _jvar_max_size)
      _jvar_map[number] = it.index();
  }

  // mark the kernel variable for the check in computeOffDiagJacobian
  _jvar_map[this->_var.number()] = 0;
}

template <class T>
unsigned int
JvarMapInterfaceBase<T>::mapJvarToCvar(unsigned int jvar)
{
  mooseAssert(jvar < _jvar_max_size,
              "Calling mapJvarToCvar for an invalid Moose variable number. Maybe an AuxVariable?");
  int cit = _jvar_map[jvar];

  mooseAssert(cit >= 0, "Calling mapJvarToCvar for a variable not coupled to this kernel.");
  return cit;
}

template <class T>
int
JvarMapInterfaceBase<T>::mapJvarToCvar(unsigned int jvar, const JvarMap & jvar_map)
{
  mooseAssert(jvar < _jvar_max_size,
              "Calling mapJvarToCvar for an invalid Moose variable number. Maybe an AuxVariable?");
  return jvar_map[jvar];
}

template <class T>
const typename JvarMapInterfaceBase<T>::JvarMap &
JvarMapInterfaceBase<T>::getParameterJvarMap(std::string parameter_name)
{
  auto & jvar_map = _jvar_local_map[parameter_name];
  jvar_map.assign(_jvar_max_size, -1);

  // populate local map
  const auto num = this->coupledComponents(parameter_name);
  for (std::size_t i = 0; i < num; ++i)
  {
    const auto number = this->getVar(parameter_name, i)->number();

    // skip AuxVars as off-diagonal jacobian entries are not calculated for them
    if (number < _jvar_max_size)
      jvar_map[number] = i;
  }

  return jvar_map;
}

template <class T>
JvarMapKernelInterface<T>::JvarMapKernelInterface(const InputParameters & parameters)
  : JvarMapInterfaceBase<T>(parameters)
{
}

template <class T>
JvarMapIntegratedBCInterface<T>::JvarMapIntegratedBCInterface(const InputParameters & parameters)
  : JvarMapInterfaceBase<T>(parameters)
{
}

template <class T>
void
JvarMapKernelInterface<T>::computeOffDiagJacobian(const unsigned int jvar)
{
  // the Kernel is not coupled to the variable; no need to loop over QPs
  if (this->_jvar_map[jvar] < 0)
    return;

  // call the underlying class' off-diagonal Jacobian
  T::computeOffDiagJacobian(jvar);
}

template <class T>
void
JvarMapIntegratedBCInterface<T>::computeOffDiagJacobian(const unsigned int jvar)
{
  // the Kernel is not coupled to the variable; no need to loop over QPs
  if (this->_jvar_map[jvar] < 0)
    return;

  // call the underlying class' off-diagonal Jacobian
  T::computeOffDiagJacobian(jvar);
}
