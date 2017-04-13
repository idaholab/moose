/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADDCOUPLEDSOLIDKINSPECIESAUXKERNELSACTION_H
#define ADDCOUPLEDSOLIDKINSPECIESAUXKERNELSACTION_H

#include "Action.h"

class AddCoupledSolidKinSpeciesAuxKernelsAction;

template <>
InputParameters validParams<AddCoupledSolidKinSpeciesAuxKernelsAction>();

class AddCoupledSolidKinSpeciesAuxKernelsAction : public Action
{
public:
  AddCoupledSolidKinSpeciesAuxKernelsAction(const InputParameters & params);

  virtual void act();

private:
  const std::vector<std::string> _reactions;
  const std::vector<Real> _logk;
  const std::vector<Real> _r_area;
  const std::vector<Real> _ref_kconst;
  const std::vector<Real> _e_act;
  const Real _gas_const;
  const std::vector<Real> _ref_temp;
  const std::vector<Real> _sys_temp;
};

#endif // ADDCOUPLEDSOLIDKINSPECIESAUXKERNELSACTION_H
