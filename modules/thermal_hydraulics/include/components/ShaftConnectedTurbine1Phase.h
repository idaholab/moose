//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VolumeJunction1Phase.h"
#include "ShaftConnectable.h"

/**
 * 1-phase turbine that must be connected to a Shaft component
 */
class ShaftConnectedTurbine1Phase : public VolumeJunction1Phase, public ShaftConnectable
{
public:
  ShaftConnectedTurbine1Phase(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;
  virtual UserObjectName getShaftConnectedUserObjectName() const override
  {
    return _junction_uo_name;
  }

protected:
  virtual void check() const override;
  virtual void buildVolumeJunctionUserObject() override;

  /// Turbine inlet
  const BoundaryName & _inlet;
  /// Turbine outlet
  const BoundaryName & _outlet;
  /// Rated turbine speed
  const Real & _omega_rated;
  /// Turbine wheel diameter
  const Real & _D_wheel;
  /// Turbine speed threshold for friction
  const Real & _speed_cr_fr;
  /// Turbine friction constant
  const Real & _tau_fr_const;
  /// Turbine friction coefficients
  const std::vector<Real> & _tau_fr_coeff;
  /// Turbine speed threshold for inertia
  const Real & _speed_cr_I;
  /// Turbine inertia constant
  const Real & _inertia_const;
  /// Turbine inertia coefficients
  const std::vector<Real> & _inertia_coeff;

  /// Name of function to compute data for turbine head
  const FunctionName & _head_coefficient;
  /// Name of function to compute data for turbine power
  const FunctionName & _power_coefficient;
  /// Name of turbine pressure drop variable
  const VariableName _delta_p_var_name;
  /// Name of turbine power variable
  const VariableName _power_var_name;
  /// Name of turbine driving torque variable
  const VariableName _driving_torque_var_name;
  /// Name of turbine friction torque variable
  const VariableName _friction_torque_var_name;
  /// Name of turbine flow_coeff torque variable
  const VariableName _flow_coeff_var_name;
  /// Name of turbine inertia variable
  const VariableName _moi_var_name;

public:
  static InputParameters validParams();
};
