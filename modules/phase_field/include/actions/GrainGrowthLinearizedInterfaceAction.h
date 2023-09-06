//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GrainGrowthAction.h"

// Forward declaration

class GrainGrowthLinearizedInterfaceAction : public GrainGrowthAction
{
public:
  static InputParameters validParams();

  GrainGrowthLinearizedInterfaceAction(const InputParameters & params);

  virtual void act();

protected:
  /// number of variables and variable name base for variable creation
  const std::string _op_name_base;
};
