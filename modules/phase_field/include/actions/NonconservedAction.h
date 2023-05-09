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
#include "Action.h"
#include "AddVariableAction.h"

#include "libmesh/fe_type.h"

class NonconservedAction : public Action
{
public:
  static InputParameters validParams();

  NonconservedAction(const InputParameters & params);

  virtual void act();

protected:
  /// Name of the variable being created
  const NonlinearVariableName _var_name;
  /// FEType for the variable being created
  const FEType _fe_type;
};
