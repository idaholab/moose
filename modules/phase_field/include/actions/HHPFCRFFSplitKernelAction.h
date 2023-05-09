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

class HHPFCRFFSplitKernelAction : public Action
{
public:
  static InputParameters validParams();

  HHPFCRFFSplitKernelAction(const InputParameters & params);

  virtual void act();

protected:
  const unsigned int _num_L;
  const std::string _L_name_base;
  const VariableName _n_name;
};
