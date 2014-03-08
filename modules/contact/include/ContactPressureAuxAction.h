#ifndef CONTACTPRESSUREAUXACTION_H
#define CONTACTPRESSUREAUXACTION_H

#include "Action.h"
#include "MooseTypes.h"
#include "MooseEnum.h"

class ContactPressureAuxAction: public Action
{
public:
  ContactPressureAuxAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  const BoundaryName _master;
  const BoundaryName _slave;
  const MooseEnum _order;
};

template<>
InputParameters validParams<ContactPressureAuxAction>();

#endif
