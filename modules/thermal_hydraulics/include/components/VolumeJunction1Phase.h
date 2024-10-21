//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowJunction1Phase.h"

/**
 * Junction between 1-phase flow channels that has a non-zero volume
 */
class VolumeJunction1Phase : public FlowJunction1Phase
{
public:
  VolumeJunction1Phase(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  /// Enumeration for junction variable/equation indices
  enum VolumeJunction1PhaseIndices
  {
    RHOV_INDEX = 0,
    RHOUV_INDEX = 1,
    RHOVV_INDEX = 2,
    RHOWV_INDEX = 3,
    RHOEV_INDEX = 4
  };
  /// Number of equations for the junction
  static const unsigned int N_EQ;

protected:
  virtual void setupMesh() override;
  virtual void check() const override;

  /**
   * Builds user object for computing and storing the fluxes
   */
  virtual void buildVolumeJunctionUserObject();

  /**
   * Returns the name of junction variable, depending on whether scalar
   *
   * @param[in] var_base  Base variable name
   */
  std::string junctionVariableName(const std::string & var_base) const;

  /**
   * Adds a junction variable to the problem, as a scalar or field variable
   *
   * @param[in] is_nonlinear  Is the variable nonlinear?
   * @param[in] var  Variable name
   * @param[in] scaling_factor  Scaling factor for variable if nonlinear
   */
  void addJunctionVariable(bool is_nonlinear, const VariableName & var, Real scaling_factor = 1.0);

  /**
   * Adds a junction IC to the problem, as a scalar or field variable
   *
   * @param[in] var  Variable name
   * @param[in] value  IC value
   */
  void addJunctionIC(const VariableName & var, Real value);

  /// True if the junction variables are scalar variables
  const bool _use_scalar_variables;

  /// Volume of the junction
  const Real _volume;

  /// Spatial position of center of the junction
  const Point & _position;

  /// Scaling factor for rho*V
  const Real & _scaling_factor_rhoV;
  /// Scaling factor for rho*u*V
  const Real & _scaling_factor_rhouV;
  /// Scaling factor for rho*v*V
  const Real & _scaling_factor_rhovV;
  /// Scaling factor for rho*w*V
  const Real & _scaling_factor_rhowV;
  /// Scaling factor for rho*E*V
  const Real & _scaling_factor_rhoEV;

  /// rho*V variable name for junction
  const VariableName _rhoV_var_name;
  /// rho*u*V variable name for junction
  const VariableName _rhouV_var_name;
  /// rho*v*V variable name for junction
  const VariableName _rhovV_var_name;
  /// rho*w*V variable name for junction
  const VariableName _rhowV_var_name;
  /// rho*E*V variable name for junction
  const VariableName _rhoEV_var_name;
  /// pressure variable name for junction
  const VariableName _pressure_var_name;
  /// temperature variable name for junction
  const VariableName _temperature_var_name;
  /// velocity variable name for junction
  const VariableName _velocity_var_name;

  /// Form loss coefficient
  const Real & _K;
  /// Reference area
  const Real & _A_ref;

public:
  static InputParameters validParams();
};
