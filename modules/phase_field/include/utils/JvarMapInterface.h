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

  /// Set the cvar value to the mapped jvar value and return true if the mapping exists.
  /// Otherwise return false
  bool mapJvarToCvar(unsigned int jvar, unsigned int & cvar);

private:
  /// look-up table to determine the _coupled_moose_vars index for the jvar parameter
  std::vector<int> _jvar_map;
};


template<class T>
JvarMapInterface<T>::JvarMapInterface(const InputParameters & parameters) :
    T(parameters),
    _jvar_map(this->_fe_problem.getNonlinearSystem().numMooseVariables(), -1)
{
  unsigned int nvar = this->_coupled_moose_vars.size();

  // populate map;
  for (unsigned int i = 0; i < nvar; ++i) {
    unsigned int number = this->_coupled_moose_vars[i]->number();

    // skip AuxVars as off-diagonal jacobian entries are not calculated for them
    if (number < _jvar_map.size())
      _jvar_map[number] = i;
  }
}


template<class T>
bool
JvarMapInterface<T>::mapJvarToCvar(unsigned int jvar, unsigned int & cvar)
{
  mooseAssert(jvar < _jvar_map.size(), "Calling mapJvarToCvar for an invalid Moose variable number. Maybe an AuxVariable?");
  int cit = _jvar_map[jvar];

  if (cit < 0)
    return false;

  cvar = cit;
  return true;
}

#endif //JVARMAPINTERFACE_H
