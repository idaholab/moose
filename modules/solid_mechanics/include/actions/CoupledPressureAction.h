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

/**
 * Action that sets up pressure boundary condition on displacement variables
 */
class CoupledPressureAction : public Action
{
public:
  static InputParameters validParams();

  CoupledPressureAction(const InputParameters & params);

  virtual void act() override;

protected:
  std::vector<std::vector<AuxVariableName>> _save_in_vars;
  std::vector<bool> _has_save_in_vars;
};
