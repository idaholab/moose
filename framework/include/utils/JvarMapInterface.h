/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef JVARMAPINTERFACE_H
#define JVARMAPINTERFACE_H

#include "MooseVariable.h"
#include "NonlinearSystem.h"

/**
 * Interface class ("Veneer") to provide a mapping from 'jvar' in
 * computeQpOffDiagJacobian into the _coupled_moose_vars array
 */
template <class T>
class JvarMapInterface : public T
{
public:
  JvarMapInterface(const InputParameters & parameters);

  void computeOffDiagJacobian(unsigned int jvar);

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
};


template<class T>
JvarMapInterface<T>::JvarMapInterface(const InputParameters & parameters) :
    T(parameters),
    _jvar_map(this->_fe_problem.getNonlinearSystem().nVariables(), -1)
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

  // mark the kernel variable for the check in computeOffDiagJacobian
  _jvar_map[this->_var.number()] = 0;
}

template<class T>
void
JvarMapInterface<T>::computeOffDiagJacobian(unsigned int jvar)
{
  // the Kernel is not coupled to the variable; no need to loop over QPs
  if (_jvar_map[jvar] < 0)
    return;

  // call the underlying class' off-diagonal Jacobian
  T::computeOffDiagJacobian(jvar);
}

template<class T>
unsigned int
JvarMapInterface<T>::mapJvarToCvar(unsigned int jvar)
{
  mooseAssert(jvar < _jvar_map.size(), "Calling mapJvarToCvar for an invalid Moose variable number. Maybe an AuxVariable?");
  int cit = _jvar_map[jvar];

  mooseAssert(cit >= 0, "Calling mapJvarToCvar for a variable not coupled to this kernel.");
  return cit;
}

template<class T>
bool
JvarMapInterface<T>::mapJvarToCvar(unsigned int jvar, unsigned int & cvar)
{
  mooseAssert(jvar < _jvar_map.size(), "Calling mapJvarToCvar for an invalid Moose variable number. Maybe an AuxVariable?");
  int cit = _jvar_map[jvar];

  if (cit < 0)
    mooseError("This should not happen!");

  cvar = cit;
  return true;
}

#endif //JVARMAPINTERFACE_H
