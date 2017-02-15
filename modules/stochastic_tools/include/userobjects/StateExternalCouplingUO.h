/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATEEXTERNALCOUPLINGUO_H
#define STATEEXTERNALCOUPLINGUO_H

#include "GeneralUserObject.h"
#include <string>

class StateExternalCouplingUO; //needed for circular reference

template <>
InputParameters validParams<StateExternalCouplingUO>();

/**
 Object for the user to assign coupled variables to be used in the StateSim model
 */
class StateExternalCouplingUO : public GeneralUserObject
{
public:
  /**
   * Main constructor to build a StateSim model.
   * @param max_sim_time - maximum total simulation time before forced stop of the simulation
   * @param ext_coupling_refs - user object for external item reference coupling.
   */
  StateExternalCouplingUO(const InputParameters & parameters);

  virtual void initialize()
  {
  }
  virtual void execute();
  virtual void finalize()
  {
  }

  /**
   * getStateExtCouplingNames - return the names of the external variables assigned.
   * @return  the names of the external variables assigned.
   */
  const std::vector<std::string> & getStateExtCouplingNames() const
  {
    return _eval_names;
  };

  /**
   * getStateExtCouplingValue - return the value of the external variables.
   * @return the value of the external variables.
   */
  Real & getStateExtCouplingValue(const int & idx) const
  {
    return (*_values)[idx];
  };

protected:
  const std::vector<std::string> & _eval_names;
  std::vector<Real> * _values;
  std::vector<Real> _no_dflt_values;
};

#endif
