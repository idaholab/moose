/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADDCOUPLEDEQSPECIESAUXKERNELSACTION_H
#define ADDCOUPLEDEQSPECIESAUXKERNELSACTION_H

#include "Action.h"

class AddCoupledEqSpeciesAuxKernelsAction;

template <>
InputParameters validParams<AddCoupledEqSpeciesAuxKernelsAction>();

class AddCoupledEqSpeciesAuxKernelsAction : public Action
{
public:
  AddCoupledEqSpeciesAuxKernelsAction(const InputParameters & params);

  virtual void act();

private:
  const std::string _reactions;
  const std::vector<std::string> _secondary_species;
};

#endif // ADDCOUPLEDEQSPECIESAUXKERNELSACTION_H
