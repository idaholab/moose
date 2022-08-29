//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"

class SinglePhaseFluidProperties;
class ADBoundaryFluxBase;

/**
 * Estimates specific impulse from fluid state at the boundary.
 * This postprocessor can estimate instantaneous and time-averaged specific impulse.
 */
class ADSpecificImpulse1Phase : public SidePostprocessor
{
public:
  ADSpecificImpulse1Phase(const InputParameters & parameters);

  virtual void threadJoin(const UserObject & y) override;
  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;

protected:
  virtual Real getValue() override;

  /// Number of components in the solution vector used to compute the flux
  const unsigned int _n_components;
  /// Variables to pass to boundary flux user object, in the correct order
  std::vector<const ADVariableValue *> _U;
  /// Boundary component name
  const std::string & _boundary_name;
  /// Boundary user object name
  const std::string _boundary_uo_name;
  /// Boundary user object
  const ADBoundaryFluxBase & _boundary_uo;
  /// the outlet pressure, user supplied value
  const Real _p_exit;
  /// the total enthalpy including mechanical energy
  const ADMaterialProperty<Real> & _H;
  /// specific volume
  const ADMaterialProperty<Real> & _v;
  /// internal energy
  const ADMaterialProperty<Real> & _e;
  /// fluid temperature
  const ADMaterialProperty<Real> & _T;
  /// fluid property user object
  const SinglePhaseFluidProperties & _fp;
  /// bisection tolerance
  const Real _tol;
  /// maximum number of iterations for bisection
  const unsigned int _max_nit = 100;
  /// if the specific impulse is accumulated over timesteps
  const bool _cumulative;
  /// accumulated mass flow rate over time
  Real & _accumulated_mass_flow_rate;
  /// accumulated thrust over time
  Real & _accumulated_thrust;

  /// total mass flow rate
  Real _mass_flow_rate;
  /// total thrust
  Real _thrust;

public:
  static InputParameters validParams();
};
