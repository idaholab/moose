//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VolumeJunction1PhaseUserObject.h"
#include "ShaftConnectableUserObjectInterface.h"

class SinglePhaseFluidProperties;
class NumericalFlux3EqnBase;

/**
 * Computes and caches flux and residual vectors for a 1-phase pump
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the pump,
 * \li fluxes between the flow channels and the pump, and
 * \li pump torque and inertia which are passed to the connected shaft.
 */
class ShaftConnectedPump1PhaseUserObject : public VolumeJunction1PhaseUserObject,
                                           public ShaftConnectableUserObjectInterface
{
public:
  ShaftConnectedPump1PhaseUserObject(const InputParameters & params);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;

  /// Hydraulic torque computed in the 1-phase shaft-connected pump
  Real getHydraulicTorque() const;
  /// Friction torque computed in the 1-phase shaft-connected pump
  Real getFrictionTorque() const;
  /// Pump head computed in the 1-phase shaft-connected pump
  Real getPumpHead() const;

  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

  virtual void getScalarEquationJacobianData(const unsigned int & equation_index,
                                             DenseMatrix<Real> & jacobian_block,
                                             std::vector<dof_id_type> & dofs_i,
                                             std::vector<dof_id_type> & dofs_j) const override;

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

  const VariableValue & _omega;

  /// Pump hydraulic torque
  Real _hydraulic_torque;
  /// Pump friction torque
  Real _friction_torque;
  /// Pump head
  Real _pump_head;

  /// Jacobian entries of junction variables wrt shaft variables
  std::vector<DenseMatrix<Real>> _residual_jacobian_omega_var;

public:
  static InputParameters validParams();
};
