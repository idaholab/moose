/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef JVARMAPINTERFACE_H
#define JVARMAPINTERFACE_H

#include "MooseVariable.h"
#include "NonlinearSystemBase.h"

template <class T>
class JvarMapInterfaceBase;

/**
 * Interface class ("Veneer") for Kernel to provide a mapping from 'jvar' in
 * computeQpOffDiagJacobian into the _coupled_moose_vars array.
 *
 * This class is useful in conjunction with DerivativeMaterialInterface,
 * where vectors of material property derviatives with respect to all coupled
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
 * where vectors of material property derviatives with respect to all coupled
 * variables (iterating over _coupled_moose_vars array) are generated.
 * The mapping enabled the look up of the correct material property derivatives
 * for the current jvar.
 */
template <class T>
class JvarMapIntegratedBCInterface : public JvarMapInterfaceBase<T>
{
public:
  JvarMapIntegratedBCInterface(const InputParameters & parameters);
  virtual void computeJacobianBlock(unsigned int jvar) override;
};

/**
 * Base class ("Veneer") that implements the actual mapping from 'jvar' in
 * into the _coupled_moose_vars array.
 */
template <class T>
class JvarMapInterfaceBase : public T
{
public:
  JvarMapInterfaceBase(const InputParameters & parameters);

  /**
   * Returns true if jvar has a corresponding index in the _coupled_moose_vars array.
   *
   * @param[in] jvar Variable number passed as argument to computeQpOffDiagJacobian
   * @return true if jvar has a corresponding index in the _coupled_moose_vars array
   */
  bool hasCvar(unsigned int jvar);

  /// Return index into the _coupled_moose_vars array for a given jvar
  unsigned int mapJvarToCvar(unsigned int jvar);

  /**
   * Set the cvar value to the mapped jvar value and return true if the mapping exists.
   * Otherwise return false.
   *
   * @param[in] jvar Variable number passed as argument to computeQpOffDiagJacobian
   * @param[out] cvar Corresponding index into the _coupled_moose_vars array
   * @return true if the requested variable is coupled, false otherwise
   */
  bool mapJvarToCvar(unsigned int jvar, unsigned int & cvar);

private:
  /// look-up table to determine the _coupled_moose_vars index for the jvar parameter
  std::vector<int> _jvar_map;

  friend class JvarMapKernelInterface<T>;
  friend class JvarMapIntegratedBCInterface<T>;
};

template <class T>
JvarMapInterfaceBase<T>::JvarMapInterfaceBase(const InputParameters & parameters)
  : T(parameters), _jvar_map(this->_fe_problem.getNonlinearSystemBase().nVariables(), -1)
{
  auto nvar = this->_coupled_moose_vars.size();

  // populate map;
  for (auto i = beginIndex(this->_coupled_moose_vars); i < nvar; ++i)
  {
    auto number = this->_coupled_moose_vars[i]->number();

    // skip AuxVars as off-diagonal jacobian entries are not calculated for them
    if (number < _jvar_map.size())
      _jvar_map[number] = i;
  }
}

template <class T>
bool
JvarMapInterfaceBase<T>::hasCvar(unsigned int jvar)
{
  return _jvar_map[jvar] >= 0;
}

template <class T>
unsigned int
JvarMapInterfaceBase<T>::mapJvarToCvar(unsigned int jvar)
{
  mooseAssert(jvar < _jvar_map.size(),
              "Calling mapJvarToCvar for an invalid Moose variable number. Maybe an AuxVariable?");
  int cit = _jvar_map[jvar];

  mooseAssert(cit >= 0, "Calling mapJvarToCvar for a variable not coupled to this kernel.");
  return cit;
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
JvarMapKernelInterface<T>::computeOffDiagJacobian(unsigned int jvar)
{
  // If the variable is not in the Kernel's _coupled_moose_vars and it is not
  // the diagonal variable, then it is known that computeQpOffDiagJacobian
  // should return zero; thus there is no need to loop over QPs. Note that
  // a Kernel may depend on the Kernel variable without that variable being
  // in _coupled_moose_vars.
  if (this->_jvar_map[jvar] < 0 && jvar != T::variable().number())
    return;

  // call the underlying class' off-diagonal Jacobian
  T::computeOffDiagJacobian(jvar);
}

template <class T>
void
JvarMapIntegratedBCInterface<T>::computeJacobianBlock(unsigned int jvar)
{
  // the Kernel is not coupled to the variable; no need to loop over QPs
  if (this->_jvar_map[jvar] < 0)
    return;

  // call the underlying class' off-diagonal Jacobian
  T::computeJacobianBlock(jvar);
}

#endif // JVARMAPINTERFACE_H
