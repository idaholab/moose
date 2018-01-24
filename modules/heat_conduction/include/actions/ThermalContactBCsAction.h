//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef THERMALCONTACTBCSACTION_H
#define THERMALCONTACTBCSACTION_H

#include "Action.h"

class ThermalContactBCsAction : public Action
{
public:
  ThermalContactBCsAction(const InputParameters & params);
  virtual ~ThermalContactBCsAction() {}
  virtual void act();
};

template <>
InputParameters validParams<ThermalContactBCsAction>();

#endif
