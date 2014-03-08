#ifndef THERMALCONTACTBCSACTION_H
#define THERMALCONTACTBCSACTION_H

#include "Action.h"

class ThermalContactBCsAction : public Action
{
public:
  ThermalContactBCsAction( const std::string & name, InputParameters params );
  virtual ~ThermalContactBCsAction(){}
  virtual void act();
};

template<>
InputParameters validParams<ThermalContactBCsAction>();

#endif
