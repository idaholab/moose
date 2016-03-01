/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSAXISYMMETRICRZACTION_H
#define TENSORMECHANICSAXISYMMETRICRZACTION_H

#include "Action.h"

class TensorMechanicsAxisymmetricRZAction;

template<>
InputParameters validParams<TensorMechanicsAxisymmetricRZAction>();

class TensorMechanicsAxisymmetricRZAction : public Action
{
public:
  TensorMechanicsAxisymmetricRZAction(const InputParameters & params);

  virtual void act();
};

#endif //TENSORMECHANICSAXISYMMETRICRZACTION_H
