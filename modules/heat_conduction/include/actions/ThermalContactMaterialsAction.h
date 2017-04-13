/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef THERMALCONTACTMATERIALSACTION_H
#define THERMALCONTACTMATERIALSACTION_H

#include "Action.h"

class ThermalContactMaterialsAction : public Action
{
public:
  ThermalContactMaterialsAction(const InputParameters & params);

  virtual void act() override;
};

template <>
InputParameters validParams<ThermalContactMaterialsAction>();

#endif
