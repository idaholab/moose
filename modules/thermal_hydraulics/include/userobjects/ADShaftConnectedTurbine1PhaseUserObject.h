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
 * Computes and caches flux and residual vectors for a 1-phase turbine
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the turbine,
 * \li fluxes between the flow channels and the turbine, and
 * \li turbine torque and inertia which are passed to the connected shaft.
 */
class ADShaftConnectedTurbine1PhaseUserObject : public ADVolumeJunction1PhaseUserObject,
                                                public ADShaftConnectableUserObjectInterface
{
public:
  ADShaftConnectedTurbine1PhaseUserObject(const InputParameters & params);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;

  /// Driving torque computed in the 1-phase shaft-connected turbine
  ADReal getDrivingTorque() const;
  /// Flow coefficient computed in the 1-phase shaft-connected turbine
  ADReal getFlowCoefficient() const;
  /// Friction torque computed in the 1-phase shaft-connected turbine
  ADReal getFrictionTorque() const;
  /// Turbine head computed in the 1-phase shaft-connected turbine
  ADReal getTurbineDeltaP() const;
  /// Turbine power computed in the 1-phase shaft-connected turbine
  ADReal getTurbinePower() const;

  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  virtual void computeFluxesAndResiduals(const unsigned int & c) override;

  /// Direction of the turbine outlet
  Point _di_out;
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
  /// Function to compute data for turbine head
  const Function & _head_coefficient;
  /// Function to compute data for turbine power
  const Function & _power_coefficient;

  /// Turbine name
  const std::string & _turbine_name;
  /// Connected shaft speed
  const ADVariableValue & _omega;

  /// Turbine driving torque
  ADReal _driving_torque;
  /// Turbine friction torque
  ADReal _friction_torque;
  /// Turbine flow coefficient - independent variable in user supplied head and power functions
  ADReal _flow_coeff;
  /// Turbine pressure drop
  ADReal _delta_p;
  /// Turbine power
  ADReal _power;

  /// Jacobian entries of junction variables wrt shaft variables
  std::vector<DenseMatrix<Real>> _residual_jacobian_omega_var;

public:
  static InputParameters validParams();
};
