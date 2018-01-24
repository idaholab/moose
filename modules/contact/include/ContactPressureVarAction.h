//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
