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

template<>
InputParameters validParams<AddCoupledEqSpeciesAuxKernelsAction>();


class AddCoupledEqSpeciesAuxKernelsAction : public Action
{
public:
  AddCoupledEqSpeciesAuxKernelsAction(const std::string & name, InputParameters params);

  virtual void act();

};

#endif // ADDCOUPLEDEQSPECIESAUXKERNELSACTION_H
