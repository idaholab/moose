/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADDCOUPLEDSOLIDKINSPECIESACTION_H
#define ADDCOUPLEDSOLIDKINSPECIESACTION_H

#include "Action.h"

class AddCoupledSolidKinSpeciesAction;

template <>
InputParameters validParams<AddCoupledSolidKinSpeciesAction>();

class AddCoupledSolidKinSpeciesAction : public Action
{
public:
  AddCoupledSolidKinSpeciesAction(const InputParameters & params);

  virtual void act() override;

  /// Prints out list of kinetic reactions
  void printReactions() const;

private:
  const std::vector<NonlinearVariableName> _primary_species;
  const std::vector<std::string> _reactions;
  const std::vector<Real> _logk;
  const std::vector<Real> _r_area;
  const std::vector<Real> _ref_kconst;
  const std::vector<Real> _e_act;
  const Real _gas_const;
  const std::vector<Real> _ref_temp;
  const std::vector<VariableName> _sys_temp;
};

#endif // ADDCOUPLEDSOLIDKINSPECIESACTION_H
