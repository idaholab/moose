//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDVECTORPOSTPROCESSORACTION_H
#define ADDVECTORPOSTPROCESSORACTION_H

#include "MooseObjectAction.h"

class AddVectorPostprocessorAction;

template <>
InputParameters validParams<AddVectorPostprocessorAction>();

class AddVectorPostprocessorAction : public MooseObjectAction
{
public:
  AddVectorPostprocessorAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDVECTORPOSTPROCESSORACTION_H
