//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

class RigidBodyMultiKernelAction : public Action
{
public:
  static InputParameters validParams();

  RigidBodyMultiKernelAction(const InputParameters & params);

  virtual void act();
  unsigned int _op_num;
  std::string _var_name_base;
  bool _implicit;
};
