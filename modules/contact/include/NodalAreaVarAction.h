/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NODALAREAVARACTION_H
#define NODALAREAVARACTION_H

#include "Action.h"
#include "MooseTypes.h"
#include "MooseEnum.h"

class NodalAreaVarAction : public Action
{
public:
  NodalAreaVarAction(const InputParameters & params);

  virtual void act();
};

template <>
InputParameters validParams<NodalAreaVarAction>();

#endif
