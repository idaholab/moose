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
 * 1-phase compressor that must be connected to a Shaft component
 */
class ShaftConnectedCompressor1Phase : public VolumeJunction1Phase, public ShaftConnectable
{
public:
  ShaftConnectedCompressor1Phase(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;
  virtual UserObjectName getShaftConnectedUserObjectName() const override
  {
    return _junction_uo_name;
  }

protected:
  virtual void check() const override;
  virtual void buildVolumeJunctionUserObject() override;

  /// Compressor inlet
  const BoundaryName & _inlet;
  /// Compressor outlet
  const BoundaryName & _outlet;
  /// Rated compressor speed
  const Real & _omega_rated;
  /// Rated compressor mass flow rate
  const Real & _mdot_rated;
  /// Rated compressor inlet stagnation fluid density
  const Real & _rho0_rated;
  /// Rated compressor inlet stagnation sound speed
  const Real & _c0_rated;
  /// Compressor speed threshold for friction
  const Real & _speed_cr_fr;
  /// Compressor friction constant
  const Real & _tau_fr_const;
  /// Compressor friction coefficients
  const std::vector<Real> & _tau_fr_coeff;
  /// Compressor speed threshold for inertia
  const Real & _speed_cr_I;
  /// Compressor inertia constant
  const Real & _inertia_const;
  /// Compressor inertia coefficients
  const std::vector<Real> & _inertia_coeff;

  /// Compressor speeds which correspond to Rp and eff function order
  const std::vector<Real> & _speeds;
  /// Names of the pressure ratio functions
  const std::vector<FunctionName> & _Rp_functions;
  /// Names of the adiabatic efficiency functions
  const std::vector<FunctionName> & _eff_functions;
  /// Name of compressor delta_p variable
  const VariableName _delta_p_var_name;
  /// Name of compressor isentropic torque variable
  const VariableName _isentropic_torque_var_name;
  /// Name of compressor dissipation torque variable
  const VariableName _dissipation_torque_var_name;
  /// Name of compressor friction torque variable
  const VariableName _friction_torque_var_name;
  /// Name of compressor inertia variable
  const VariableName _moi_var_name;

public:
  static InputParameters validParams();
};
