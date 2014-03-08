#ifndef THERMALCONTACTAUXBCSACTION_H
#define THERMALCONTACTAUXBCSACTION_H

#include "Action.h"

class ThermalContactAuxBCsAction : public Action
{
public:
  ThermalContactAuxBCsAction(const std::string & name, InputParameters params);
  virtual ~ThermalContactAuxBCsAction(){}
  virtual void act();
};

template<>
InputParameters validParams<ThermalContactAuxBCsAction>();

#endif
