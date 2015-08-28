/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DYNAMICTENSORMECHANICSACTION_H
#define DYNAMICTENSORMECHANICSACTION_H

#include "Action.h"

class DynamicTensorMechanicsAction;

template<>
InputParameters validParams<DynamicTensorMechanicsAction>();

class DynamicTensorMechanicsAction : public Action
{
public:
  DynamicTensorMechanicsAction(const InputParameters & params);

  virtual void act();

private:
 const Real _zeta;
 const Real _alpha;
};

#endif //DYNAMICTENSORMECHANICSACTION_H
