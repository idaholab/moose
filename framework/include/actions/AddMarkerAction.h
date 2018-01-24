//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDMARKERACTION_H
#define ADDMARKERACTION_H

#include "MooseObjectAction.h"

class AddMarkerAction;

template <>
InputParameters validParams<AddMarkerAction>();

class AddMarkerAction : public MooseObjectAction
{
public:
  AddMarkerAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDMARKERACTION_H
