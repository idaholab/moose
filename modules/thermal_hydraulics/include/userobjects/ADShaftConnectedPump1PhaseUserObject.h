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
#include "ADWeightedTransition.h"

class SinglePhaseFluidProperties;
class ADNumericalFlux3EqnBase;

/**
 * Computes and caches flux and residual vectors for a 1-phase pump
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the pump,
 * \li fluxes between the flow channels and the pump, and
 * \li pump torque and inertia which are passed to the connected shaft.
 */
class ADShaftConnectedPump1PhaseUserObject : public ADVolumeJunction1PhaseUserObject,
                                             public ADShaftConnectableUserObjectInterface
{
public:
  ADShaftConnectedPump1PhaseUserObject(const InputParameters & params);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;

  /// Hydraulic torque computed in the 1-phase shaft-connected pump
  ADReal getHydraulicTorque() const;
  /// Friction torque computed in the 1-phase shaft-connected pump
  ADReal getFrictionTorque() const;
  /// Pump head computed in the 1-phase shaft-connected pump
  ADReal getPumpHead() const;

  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  virtual void computeFluxesAndResiduals(const unsigned int & c) override;

  /// Direction of the pump outlet
  Point _di_out;
  /// Gravity constant
  const Real & _g;
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
  /// Function to compute data for pump head
  const Function & _head;
  /// Function to compute data for pump torque
  const Function & _torque_hydraulic;
  /// Pump name
  const std::string & _pump_name;
  /// Shaft speed
  const ADVariableValue & _omega;
  /// Transition width for the sign of the frictional torque when speed is 0
  const Real & _transition_width;
  /// Transition for the sign of the frictional torque when speed is 0
  const ADWeightedTransition _transition_friction;

  /// Pump hydraulic torque
  ADReal _hydraulic_torque;
  /// Pump friction torque
  ADReal _friction_torque;
  /// Pump head
  ADReal _pump_head;

  /// Jacobian entries of junction variables wrt shaft variables
  std::vector<DenseMatrix<Real>> _residual_jacobian_omega_var;

public:
  static InputParameters validParams();
};
