//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "MooseTypes.h"

/**
 * Action that adds default SubChannel geometry initial conditions
 */
class SubChannelAddInitialConditionsAction : public Action
{
public:
  static InputParameters validParams();

  SubChannelAddInitialConditionsAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /// Add an initial condition unless another IC already targets the variable
  void addInitialCondition(const std::string & type,
                           const std::string & name,
                           const VariableName & var_name);

  /// Return whether any user-provided IC targets the variable
  bool hasInitialCondition(const VariableName & var_name) const;
};
