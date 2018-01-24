//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
