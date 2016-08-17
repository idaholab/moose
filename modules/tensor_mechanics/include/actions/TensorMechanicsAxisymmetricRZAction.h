/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSAXISYMMETRICRZACTION_H
#define TENSORMECHANICSAXISYMMETRICRZACTION_H

#include "TensorMechanicsAction.h"

class TensorMechanicsAxisymmetricRZAction;

template<>
InputParameters validParams<TensorMechanicsAxisymmetricRZAction>();

class TensorMechanicsAxisymmetricRZAction : public TensorMechanicsAction
{
public:
  TensorMechanicsAxisymmetricRZAction(const InputParameters & params);
};

#endif //TENSORMECHANICSAXISYMMETRICRZACTION_H
