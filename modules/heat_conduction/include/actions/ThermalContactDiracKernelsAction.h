#ifndef THERMALCONTACTDIRACKERNELSACTION_H
#define THERMALCONTACTDIRACKERNELSACTION_H

#include "Action.h"

class ThermalContactDiracKernelsAction : public Action
{
public:
  ThermalContactDiracKernelsAction(const std::string & name, InputParameters params);
  virtual ~ThermalContactDiracKernelsAction(){}
  virtual void act();
};

template<>
InputParameters validParams<ThermalContactDiracKernelsAction>();

#endif
