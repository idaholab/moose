//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "JacobianTestAction.h"
#include "MooseEnum.h"

/**
 * Sets up a Jacobian test for 1-phase rDG
 */
class JacobianTest1PhaseRDGAction : public JacobianTestAction
{
public:
  JacobianTest1PhaseRDGAction(const InputParameters & params);

protected:
  virtual void addObjects() override;
  virtual void addMesh() override;
  virtual void addInitialConditions() override;
  virtual void addSolutionVariables() override;
  virtual void addAuxVariables() override;
  virtual void addMaterials() override;
  virtual void addUserObjects() override;

  /**
   * Adds solution variables with Riemann problem IC (constant left and right states)
   *
   * @param[in] variables  names of the solution variables
   * @param[in] values_left  values of the solution variables on the left half
   * @param[in] values_right  values of the solution variables on the right half
   */
  void addSolutionVariablesRiemannIC(const std::vector<VariableName> & variables,
                                     const std::vector<Real> & values_left,
                                     const std::vector<Real> & values_right);

  /// cross-sectional area variable name, elemental average
  const VariableName _A_name;
  /// cross-sectional area variable name, linear Lagrange
  const VariableName _A_linear_name;
  /// rho*A variable name
  const VariableName _rhoA_name;
  /// rho*u*A variable name
  const VariableName _rhouA_name;
  /// rho*E*A variable name
  const VariableName _rhoEA_name;

  /// option to add DG kernel
  const bool _add_dg_kernel;
  /// option to BC
  const bool _add_bc;

  /// name of numerical flux user object being tested
  const UserObjectName _numerical_flux_name;
  /// name of boundary flux user object being tested
  const UserObjectName _boundary_flux_name;

  /// initial conditions option
  const MooseEnum _ic_option;
  /// area function name
  const FunctionName & _A_fn_name;

  /// option to use slope reconstruction
  const bool _use_slope_reconstruction;

  /// reconstruction material name
  const std::string _reconstruction_material_name;

  /// direction material property name
  const MaterialPropertyName _direction_name;

  /// fluid properties object name
  const UserObjectName _fp_name;

public:
  static InputParameters validParams();
};
