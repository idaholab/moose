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
 * Computes and caches flux and residual vectors for a 1-phase compressor
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the compressor,
 * \li fluxes between the flow channels and the compressor, and
 * \li compressor torque and inertia which are passed to the connected shaft.
 */
class ADShaftConnectedCompressor1PhaseUserObject : public ADVolumeJunction1PhaseUserObject,
                                                   public ADShaftConnectableUserObjectInterface
{
public:
  ADShaftConnectedCompressor1PhaseUserObject(const InputParameters & params);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;

  /// Isentropic torque computed in the 1-phase shaft-connected compressor
  ADReal getIsentropicTorque() const;
  /// Dissipation torque computed in the 1-phase shaft-connected compressor
  ADReal getDissipationTorque() const;
  /// Friction torque computed in the 1-phase shaft-connected compressor
  ADReal getFrictionTorque() const;
  /// Compressor head computed in the 1-phase shaft-connected compressor
  ADReal getCompressorDeltaP() const;
  /// Gets the pressure ratio
  ADReal getPressureRatio() const;
  /// Gets the efficiency
  ADReal getEfficiency() const;
  /// Gets the relative corrected mass flow rate
  ADReal getRelativeCorrectedMassFlowRate() const;
  /// Gets the elative corrected shaft speed
  ADReal getRelativeCorrectedSpeed() const;

  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  virtual void computeFluxesAndResiduals(const unsigned int & c) override;

  /// Direction of the compressor outlet
  Point _di_out;
  /// Treat the compressor as a turbine?
  const bool _treat_as_turbine;
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

  /// Compressor name
  const std::string & _compressor_name;

  const ADVariableValue & _omega;

  /// Compressor isentropic torque
  ADReal _isentropic_torque;
  /// Compressor dissipation torque
  ADReal _dissipation_torque;
  /// Compressor friction torque
  ADReal _friction_torque;
  /// Compressor delta p
  ADReal _delta_p;
  /// Pressure ratio
  ADReal _Rp;
  /// Efficiency
  ADReal _eff;
  /// relative corrected mass flow rate
  ADReal _flow_rel_corr;
  /// relative corrected shaft speed
  ADReal _speed_rel_corr;

  /// Jacobian entries of junction variables wrt shaft variables
  std::vector<DenseMatrix<Real>> _residual_jacobian_omega_var;

public:
  static InputParameters validParams();
};
