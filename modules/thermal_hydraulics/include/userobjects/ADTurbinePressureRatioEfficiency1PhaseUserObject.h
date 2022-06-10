//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVolumeJunction1PhaseUserObject.h"
#include "ADShaftConnectableUserObjectInterface.h"

class SinglePhaseFluidProperties;
class ADNumericalFlux3EqnBase;

/**
 * Computes and caches flux and residual vectors for a 1-phase Turbine
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the Turbine,
 * \li fluxes between the flow channels and the Turbine, and
 * \li Turbine torque and inertia which are passed to the connected shaft.
 */
class ADTurbinePressureRatioEfficiency1PhaseUserObject
  : public ADVolumeJunction1PhaseUserObject,
    public ADShaftConnectableUserObjectInterface
{
public:
  ADTurbinePressureRatioEfficiency1PhaseUserObject(const InputParameters & params);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;

  /// Isentropic torque computed in the 1-phase shaft-connected Turbine
  ADReal getIsentropicTorque() const;
  /// Dissipation torque computed in the 1-phase shaft-connected Turbine
  ADReal getDissipationTorque() const;
  /// Friction torque computed in the 1-phase shaft-connected Turbine
  ADReal getFrictionTorque() const;
  /// Turbine head computed in the 1-phase shaft-connected Turbine
  ADReal getTurbineDeltaP() const;
  /// Gets the pressure ratio
  ADReal getPressureRatio() const;
  /// Gets the efficiency
  ADReal getEfficiency() const;

  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  virtual void computeFluxesAndResiduals(const unsigned int & c) override;

  /// Direction of the Turbine outlet
  Point _di_out;
  /// Rated Turbine speed
  const Real & _omega_rated;
  /// Rated Turbine mass flow rate
  const Real & _mdot_rated;
  /// Rated Turbine inlet stagnation fluid density
  const Real & _rho0_rated;
  /// Rated Turbine inlet stagnation sound speed
  const Real & _c0_rated;
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
  /// Turbine speeds which correspond to Rp and eff function order
  const std::vector<Real> & _speeds;
  /// Names of the pressure ratio functions
  const std::vector<FunctionName> & _Rp_function_names;
  /// Names of the adiabatic efficiency functions
  const std::vector<FunctionName> & _eff_function_names;
  /// Size of vector _speeds
  const unsigned int _n_speeds;
  /// Pressure ratio functions
  std::vector<const Function *> _Rp_functions;
  /// Adiabatic efficiency functions
  std::vector<const Function *> _eff_functions;
  /// Minimum pressure ratio
  const Real & _Rp_min;
  /// Maximum pressure ratio
  const Real & _Rp_max;

  /// Turbine name
  const std::string & _turbine_name;

  const ADVariableValue & _omega;

  /// Turbine isentropic torque
  ADReal _isentropic_torque;
  /// Turbine dissipation torque
  ADReal _dissipation_torque;
  /// Turbine friction torque
  ADReal _friction_torque;
  /// Turbine delta p
  ADReal _delta_p;
  /// Pressure ratio
  ADReal _Rp;
  /// Efficiency
  ADReal _eff;

  /// Jacobian entries of junction variables wrt shaft variables
  std::vector<DenseMatrix<Real>> _residual_jacobian_omega_var;

public:
  static InputParameters validParams();
};
