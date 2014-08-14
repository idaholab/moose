#ifndef CONTACTPENETRATIONVARACTION_H
#define CONTACTPENETRATIONVARACTION_H

#include "Action.h"
#include "MooseTypes.h"

class ContactPenetrationVarAction: public Action
{
public:
  ContactPenetrationVarAction(const std::string & name, InputParameters params);

  virtual void act();

};

template<>
InputParameters validParams<ContactPenetrationVarAction>();

#endif
