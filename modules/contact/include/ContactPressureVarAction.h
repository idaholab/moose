#ifndef CONTACTPRESSUREVARACTION_H
#define CONTACTPRESSUREVARACTION_H

#include "Action.h"
#include "MooseTypes.h"
#include "MooseEnum.h"

class ContactPressureVarAction: public Action
{
public:
  ContactPressureVarAction(const std::string & name, InputParameters params);

  virtual void act();

};

template<>
InputParameters validParams<ContactPressureVarAction>();

#endif
