//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowModelSetup.h"

/**
 * Helper class to set up some objects for 1-phase flow
 */
class FlowModelSetup1Phase : public FlowModelSetup
{
public:
  FlowModelSetup1Phase(const InputParameters & params);

protected:
  virtual void addInitialConditions() override;
  virtual void addSolutionVariables() override;
  virtual void addNonConstantAuxVariables() override;
  virtual void addMaterials() override;
  virtual void addUserObjects() override;

  /// Pressure function name
  const FunctionName _p_fn;
  /// Temperature function name
  const FunctionName _T_fn;
  /// Velocity function name
  const FunctionName _vel_fn;
  /// Area function name
  const FunctionName _A_fn;
  /// Hydraulic diameter function name
  const FunctionName _D_h_fn;

  /// Single-phase fluid properties object name
  const UserObjectName _fp_1phase_name;

  /// Name of unity
  const VariableName _unity_name;
  /// Name of area variable
  const VariableName _A_name;
  /// Name of the hydraulic diameter
  const VariableName _D_h_name;
  /// Name of rho*A variable
  const VariableName _rhoA_name;
  /// Name of rho*u*A variable
  const VariableName _rhouA_name;
  /// Name of rho*E*A variable
  const VariableName _rhoEA_name;
  /// Name of rho variable
  const VariableName _rho_name;
  /// Name of velocity variable
  const VariableName _vel_name;
  /// Name of pressure variable
  const VariableName _p_name;
  /// Name of temperature variable
  const VariableName _T_name;
  /// Name of specific volume variable
  const VariableName _v_name;
  /// Name of specific internal energy variable
  const VariableName _e_name;
  /// Name of specific total enthalpy variable
  const VariableName _H_name;
  /// Name of dynamic viscosity variable
  const VariableName _mu_name;

  /// True for setting up testing with AD, false otherwise
  const bool & _ad;

public:
  static InputParameters validParams();
};
