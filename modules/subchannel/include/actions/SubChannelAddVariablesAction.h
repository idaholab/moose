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
#include "MooseEnum.h"
#include "MooseTypes.h"

/**
 * Action that adds SubChannel variables needs for the solve
 */
class SubChannelAddVariablesAction : public Action
{
public:
  static InputParameters validParams();

  SubChannelAddVariablesAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /// Add a block-restricted auxiliary variable unless the user already defined it
  void addAuxVariable(const std::string & var_name, const std::vector<SubdomainName> & blocks);

  /// FE family of the aux variables added by this action
  const MooseEnum _fe_family;
  /// FE order of the aux variables added by this action
  const MooseEnum _fe_order;
};
