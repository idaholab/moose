#ifndef THERMALCONTACTAUXVARSACTION_H
#define THERMALCONTACTAUXVARSACTION_H

#include "Action.h"

class ThermalContactAuxVarsAction : public Action
{
public:
  ThermalContactAuxVarsAction(const std::string & name, InputParameters params);
  virtual ~ThermalContactAuxVarsAction(){}
  virtual void act();

  static std::string
  getGapValueName(const InputParameters & param)
  {
    return "paired_" + param.get<NonlinearVariableName>("variable");
  }

  static std::string
  getGapConductivityName(const InputParameters & param)
  {
    return "paired_k_" + param.get<NonlinearVariableName>("variable");
  }

};

template<>
InputParameters validParams<ThermalContactAuxVarsAction>();

#endif
