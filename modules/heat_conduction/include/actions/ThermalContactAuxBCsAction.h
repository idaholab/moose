//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef THERMALCONTACTAUXBCSACTION_H
#define THERMALCONTACTAUXBCSACTION_H

#include "Action.h"

class ThermalContactAuxBCsAction : public Action
{
public:
  ThermalContactAuxBCsAction(const InputParameters & params);
  virtual ~ThermalContactAuxBCsAction() {}
  virtual void act();
};

template <>
InputParameters validParams<ThermalContactAuxBCsAction>();

#endif
