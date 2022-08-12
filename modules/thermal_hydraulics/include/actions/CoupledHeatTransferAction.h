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

#include "MooseEnum.h"

/**
 * Action that creates the necessary objects, for the solid side, to couple a
 * solid heat conduction region to a 1-D flow channel via convective heat transfer
 */
class CoupledHeatTransferAction : public Action
{
public:
  static InputParameters validParams();

  CoupledHeatTransferAction(const InputParameters & params);
  virtual void act() override;

protected:
  virtual void addBCs();
  virtual void addUserObjects();
  virtual void addTransfers();

  /// Boundary where the BC is applied
  std::vector<BoundaryName> _boundary;

  /// Solid side temperature variable name
  const VariableName _T_solid_var_name;

  /// Variable on the flow channel side into which to transfer the solid temperature
  const VariableName _T_wall_var_name;

  /// Variable(s) on the solid side into which to transfer the fluid temperature(s)
  const std::vector<VariableName> _T_fluid_var_names;

  /// Variable(s) on the solid side into which to transfer the heat transfer coefficient(s)
  const std::vector<VariableName> _htc_var_names;

  /// Variables on the solid side into which to transfer the wall contact fractions
  std::vector<VariableName> _kappa_var_names;

  /// Number of fluid phases
  const unsigned int _n_phases;

  /// User object name with solid temperature
  const UserObjectName _T_wall_user_object_name;

  /// Spatial user object(s) holding the fluid temperature values
  std::vector<UserObjectName> _T_fluid_user_object_names;

  /// Spatial user object(s) holding the heat transfer coefficient values
  std::vector<UserObjectName> _htc_user_object_names;

  /// Spatial user objects holding the wall contact fraction values
  std::vector<UserObjectName> _kappa_user_object_names;

  /// Name of the THM multi-app
  MultiAppName _multi_app_name;
};
