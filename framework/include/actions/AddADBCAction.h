//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDADBCACTION_H
#define ADDADBCACTION_H

#include "MooseADObjectAction.h"

class AddADBCAction;

template <>
InputParameters validParams<AddADBCAction>();

class AddADBCAction : public MooseADObjectAction
{
public:
  AddADBCAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDADBCACTION_H
