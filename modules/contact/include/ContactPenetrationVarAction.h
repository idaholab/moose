/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONTACTPENETRATIONVARACTION_H
#define CONTACTPENETRATIONVARACTION_H

#include "Action.h"
#include "MooseTypes.h"

class ContactPenetrationVarAction : public Action
{
public:
  ContactPenetrationVarAction(const InputParameters & params);

  virtual void act();
};

template <>
InputParameters validParams<ContactPenetrationVarAction>();

#endif
