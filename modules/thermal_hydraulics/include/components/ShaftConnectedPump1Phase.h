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
 * 1-phase pump that must be connected to a Shaft component
 */
class ShaftConnectedPump1Phase : public VolumeJunction1Phase, public ShaftConnectable
{
public:
  ShaftConnectedPump1Phase(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;
  virtual UserObjectName getShaftConnectedUserObjectName() const override
  {
    return _junction_uo_name;
  }

protected:
  virtual void check() const override;
  virtual void buildVolumeJunctionUserObject() override;

  /// Pump inlet
  const BoundaryName & _inlet;
  /// Pump outlet
  const BoundaryName & _outlet;
  /// Rated pump speed
  const Real & _omega_rated;
  /// Rated pump volumetric flow rate
  const Real & _volumetric_rated;
  /// Rated pump head
  const Real & _head_rated;
  /// Rated pump torque
  const Real & _torque_rated;
  /// Rated pump density
  const Real & _density_rated;
  /// Pump speed threshold for friction
  const Real & _speed_cr_fr;
  /// Pump friction constant
  const Real & _tau_fr_const;
  /// Pump friction coefficients
  const std::vector<Real> & _tau_fr_coeff;
  /// Pump speed threshold for inertia
  const Real & _speed_cr_I;
  /// Pump inertia constant
  const Real & _inertia_const;
  /// Pump inertia coefficients
  const std::vector<Real> & _inertia_coeff;
  /// Name of function to compute data for pump head
  const FunctionName & _head;
  /// Name of function to compute data for pump torque
  const FunctionName & _torque_hydraulic;
  /// Name of pump head variable
  const VariableName _head_var_name;
  /// Name of pump hydraulic torque variable
  const VariableName _hydraulic_torque_var_name;
  /// Name of pump friction torque variable
  const VariableName _friction_torque_var_name;
  /// Name of pump inertia variable
  const VariableName _moi_var_name;
  /// Transition width for the sign of the frictional torque when speed is 0
  const Real & _transition_width;

public:
  static InputParameters validParams();
};
