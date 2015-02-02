#ifndef JVARMAPINTERFACE_H
#define JVARMAPINTERFACE_H

#include "MooseVariable.h"
#include "MooseVariableMap.h"

/**
 * Interface class ("Veneer") to provide a mapping from 'jvar' in
 * computeQpOffDiagJacobian into the _coupled_moose_vars array
 */
template <class T>
class JvarMapInterface : public T
{
public:
  JvarMapInterface(const std::string & name, InputParameters parameters);

  /// Set the cvar value to the mapped jvar value and return true if the mapping exists.
  /// Otherwise return false
  bool mapJvarToCvar(unsigned int jvar, unsigned int & cvar);

protected:
  /// map type for brevity
  typedef std::map<unsigned int, unsigned int> JvarMap;

  /// look-up table to determine the _coupled_moose_vars index for the jvar parameter
  MooseVariableMap<int> _jvar_map;
};


template<class T>
JvarMapInterface<T>::JvarMapInterface(const std::string & name, InputParameters parameters) :
    T(name, parameters),
    _jvar_map(this->_fe_problem, -1)
{
  unsigned int nvar = this->_coupled_moose_vars.size();

  // populate map
  for (unsigned int i = 0; i < nvar; ++i)
    _jvar_map.insert(std::pair<unsigned int, int>(this->_coupled_moose_vars[i]->number(), i));
}

template<class T>
bool
JvarMapInterface<T>::mapJvarToCvar(unsigned int jvar, unsigned int & cvar)
{
  int c = _jvar_map[jvar];
  if (c < 0) return false;

  cvar = c;
  return true;
}

#endif //JVARMAPINTERFACE_H
