/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONTACTPENETRATIONAUXACTION_H
#define CONTACTPENETRATIONAUXACTION_H

#include "Action.h"
#include "MooseTypes.h"
#include "MooseEnum.h"

class ContactPenetrationAuxAction;

template <>
InputParameters validParams<ContactPenetrationAuxAction>();

class ContactPenetrationAuxAction : public Action
{
public:
  ContactPenetrationAuxAction(const InputParameters & params);

  virtual void act();

private:
  const BoundaryName _master;
  const BoundaryName _slave;
  const MooseEnum _order;
};

#endif // CONTACTACTION_H
