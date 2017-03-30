/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
