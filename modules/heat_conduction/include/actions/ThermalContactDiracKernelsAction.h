/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef THERMALCONTACTDIRACKERNELSACTION_H
#define THERMALCONTACTDIRACKERNELSACTION_H

#include "Action.h"

class ThermalContactDiracKernelsAction : public Action
{
public:
  ThermalContactDiracKernelsAction(const InputParameters & params);
  virtual ~ThermalContactDiracKernelsAction() {}
  virtual void act();
};

template <>
InputParameters validParams<ThermalContactDiracKernelsAction>();

#endif
