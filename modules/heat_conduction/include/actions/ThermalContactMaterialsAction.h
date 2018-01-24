//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
