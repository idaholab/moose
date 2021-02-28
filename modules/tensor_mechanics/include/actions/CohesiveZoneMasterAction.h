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

class CohesiveZoneMasterAction : public Action
{
public:
  static InputParameters validParams();
  CohesiveZoneMasterAction(const InputParameters & params);

  /// strain formulation
  enum class Kinematic
  {
    SmallStrain,
    TotalLagrangian
  } _kinematic;

  /// Method adding the proper relationship manager
  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

  void act() override;

protected:
  std::string _czm_kernel_name;
  std::string _disp_jump_provider_name;
  std::string _equilibrium_traction_calculator_name;
};
