/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADDCOUPLEDEQSPECIESKERNELSACTION_H
#define ADDCOUPLEDEQSPECIESKERNELSACTION_H

#include "Action.h"

class AddCoupledEqSpeciesKernelsAction;

template <>
InputParameters validParams<AddCoupledEqSpeciesKernelsAction>();

class AddCoupledEqSpeciesKernelsAction : public Action
{
public:
  AddCoupledEqSpeciesKernelsAction(const InputParameters & params);

  virtual void act();
};

#endif // ADDCOUPLEDEQSPECIESKERNELSACTION_H
