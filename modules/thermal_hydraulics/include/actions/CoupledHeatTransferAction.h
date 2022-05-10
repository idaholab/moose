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
 * Action that controls the creation of all of the necessary objects for
 * doing transfer between heat conduction and thermal hydraulics module
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

  /// The name of the solid temperature variable
  VariableName _solid_temp_var_name;

  /// The name of the fluid temperature variable
  VariableName _fluid_temp_var_name;

  /// The name of the wall temperature variable in THM
  VariableName _wall_temp_var_name;

  /// The name of the heat transfer coefficient variable
  VariableName _htc_var_name;

  /// The MooseEnum direction the layers are going in
  MooseEnum _direction_enum;

  /// Number of layers to split the mesh into
  unsigned int _num_layers;

  /// User object name with solid temperature
  UserObjectName _T_avg_user_object_name;

  /// User object name with fluid temperature
  UserObjectName _th_T_fluid_user_object_name;

  /// User object name with heat transfer temperature
  UserObjectName _th_htc_user_object_name;

  /// Name of the THM multi-app
  MultiAppName _multi_app_name;
};
