/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NODALAREAACTION_H
#define NODALAREAACTION_H

#include "MooseObjectAction.h"
#include "MooseTypes.h"
#include "MooseEnum.h"

class NodalAreaAction : public MooseObjectAction
{
public:
  NodalAreaAction(const InputParameters & params);

  virtual void act();
};

template <>
InputParameters validParams<NodalAreaAction>();

#endif
