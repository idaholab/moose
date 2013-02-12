#ifndef THERMALCONTACTMATERIALSACTION_H
#define THERMALCONTACTMATERIALSACTION_H

#include "Action.h"

class ThermalContactMaterialsAction : public Action
{
public:
  ThermalContactMaterialsAction( const std::string & name, InputParameters params );
  virtual ~ThermalContactMaterialsAction(){}
  virtual void act();
};

template<>
InputParameters validParams<ThermalContactMaterialsAction>();

#endif
