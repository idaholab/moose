//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef THERMALCONTACTAUXVARSACTION_H
#define THERMALCONTACTAUXVARSACTION_H

#include "Action.h"

class ThermalContactAuxVarsAction : public Action
{
public:
  ThermalContactAuxVarsAction(const InputParameters & params);

  virtual void act() override;

  static std::string getGapValueName(const InputParameters & param)
  {
    return "paired_" + param.get<NonlinearVariableName>("variable");
  }

  static std::string getGapConductivityName(const InputParameters & param)
  {
    return "paired_k_" + param.get<NonlinearVariableName>("variable");
  }
};

template <>
InputParameters validParams<ThermalContactAuxVarsAction>();

#endif
