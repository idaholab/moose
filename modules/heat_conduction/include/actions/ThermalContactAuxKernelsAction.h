#ifndef THERMALCONTACTAUXKERNELSACTION_H
#define THERMALCONTACTAUXKERNELSACTION_H

#include "Action.h"

class ThermalContactAuxKernelsAction : public Action
{
public:
  ThermalContactAuxKernelsAction(const std::string & name, InputParameters params);
  virtual ~ThermalContactAuxKernelsAction(){}
  virtual void act();
};

template<>
InputParameters validParams<ThermalContactAuxKernelsAction>();

#endif
