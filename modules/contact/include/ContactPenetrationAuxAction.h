#ifndef CONTACTPENETRATIONAUXACTION_H
#define CONTACTPENETRATIONAUXACTION_H

#include "Action.h"
#include "MooseTypes.h"
#include "MooseEnum.h"

class ContactPenetrationAuxAction;

template<>
InputParameters validParams<ContactPenetrationAuxAction>();

class ContactPenetrationAuxAction: public Action
{
public:
  ContactPenetrationAuxAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  const BoundaryName _master;
  const BoundaryName _slave;
  const MooseEnum _order;
};


#endif // CONTACTACTION_H
