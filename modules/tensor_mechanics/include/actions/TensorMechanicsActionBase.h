/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSACTIONBASE_H
#define TENSORMECHANICSACTIONBASE_H

#include "Action.h"

class TensorMechanicsActionBase;

template <>
InputParameters validParams<TensorMechanicsActionBase>();

class TensorMechanicsActionBase : public Action
{
public:
  TensorMechanicsActionBase(const InputParameters & params);
};

#endif // TENSORMECHANICSACTIONBASE_H
