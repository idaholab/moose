/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
