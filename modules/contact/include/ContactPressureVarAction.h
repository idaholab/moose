/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONTACTPRESSUREVARACTION_H
#define CONTACTPRESSUREVARACTION_H

#include "Action.h"
#include "MooseTypes.h"

class ContactPressureVarAction : public Action
{
public:
  ContactPressureVarAction(const InputParameters & params);

  virtual void act();
};

template <>
InputParameters validParams<ContactPressureVarAction>();

#endif
