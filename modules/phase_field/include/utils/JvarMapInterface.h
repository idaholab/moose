#ifndef JVARMAPINTERFACE_H
#define JVARMAPINTERFACE_H

#include "MooseVariable.h"

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
  JvarMap _jvar_map;
};


template<class T>
JvarMapInterface<T>::JvarMapInterface(const std::string & name, InputParameters parameters) :
    T(name, parameters)
{
  unsigned int nvar = this->_coupled_moose_vars.size();

  // populate map;
  for (unsigned int i = 0; i < nvar; ++i)
    _jvar_map[this->_coupled_moose_vars[i]->number()] = i;
}

template<class T>
bool
JvarMapInterface<T>::mapJvarToCvar(unsigned int jvar, unsigned int & cvar)
{
  JvarMap::iterator cit = _jvar_map.find(jvar);

  if (cit == _jvar_map.end())
    return false;

  cvar = cit->second;
  return true;
}

#endif //JVARMAPINTERFACE_H
