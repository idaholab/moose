//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelScalarBase.h"
#include "DerivativeMaterialInterface.h"
#include "JvarMapInterface.h"
#include "StabilizationUtils.h"
#include "RankFourTensorForward.h"

/// Base class of the "Lagrangian" kernel system
///
/// This class provides a common structure for the "new" tensor_mechanics
/// kernel system.  The goals for this new system are
///   1) Always-correct jacobians
///   2) A cleaner material interface
///
/// This class provides common input properties and helper methods,
/// most of the math has to be done in the subclasses
///
class LagrangianStressDivergenceBase
  : public JvarMapKernelInterface<DerivativeMaterialInterface<KernelScalarBase>>
{
public:
  static InputParameters validParams();
  LagrangianStressDivergenceBase(const InputParameters & parameters);

protected:
  // Helper function to return the test function gradient which may depend on kinematics and
  // stabilization
  virtual RankTwoTensor gradTest(unsigned int component) = 0;

  // Helper function to return the trial function gradient which may depend on kinematics and
  // stabilization
  virtual RankTwoTensor gradTrial(unsigned int component) = 0;

  virtual void precalculateJacobian() override;
  virtual void precalculateOffDiagJacobian(unsigned int jvar) override;

  /// Prepare the average shape function gradients for stabilization
  virtual void precalculateJacobianDisplacement(unsigned int component) = 0;

  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  // Derivatives of the residual w.r.t. the displacement dofs
  virtual Real computeQpJacobianDisplacement(unsigned int alpha, unsigned int beta) = 0;

  // Derivatives of the residual w.r.t. the temperature dofs through eigenstrain
  virtual Real computeQpJacobianTemperature(unsigned int cvar) = 0;

  // Derivatives of the residual w.r.t. the out-of-plane strain
  virtual Real computeQpJacobianOutOfPlaneStrain() = 0;

protected:
  /// If true use large deformation kinematics
  const bool _large_kinematics;

  /// If true calculate the deformation gradient derivatives for F_bar
  const bool _stabilize_strain;

  /// Prepend to the material properties
  const std::string _base_name;

  /// Which component of the vector residual this kernel is responsible for
  const unsigned int _alpha;

  /// Total number of displacements/size of residual vector
  const unsigned int _ndisp;

  /// The displacement numbers
  std::vector<unsigned int> _disp_nums;

  // Averaged trial function gradients for each displacement component
  // i.e. _avg_grad_trial[a][j] returns the average gradient of trial function associated with
  // node j with respect to displacement component a.
  std::vector<std::vector<RankTwoTensor>> _avg_grad_trial;

  /// The unmodified deformation gradient
  const MaterialProperty<RankTwoTensor> & _F_ust;

  /// The element-average deformation gradient
  const MaterialProperty<RankTwoTensor> & _F_avg;

  /// The inverse increment deformation gradient
  const MaterialProperty<RankTwoTensor> & _f_inv;

  /// The inverse deformation gradient
  const MaterialProperty<RankTwoTensor> & _F_inv;

  /// The actual (stabilized) deformation gradient. With the generalized midpoint rule this is the
  /// alpha-weighted F, NOT the literal F at n+1.
  const MaterialProperty<RankTwoTensor> & _F;

  /// The literal deformation gradient at n+1 (I + grad u_{n+1}), independent of alpha and F-bar.
  /// Used by the UL kernel to convert spatial gradients to reference-frame gradients.
  const MaterialProperty<RankTwoTensor> & _F_actual;

  /// Derivative of the spatial velocity gradient increment w.r.t. F_{n+1}
  const MaterialProperty<RankFourTensor> & _d_spatial_velocity_increment_d_F;

  /// Derivative of F_{n+1} w.r.t. the displacement gradient
  const MaterialProperty<RankFourTensor> & _d_F_d_grad_u;

  /// Partials of the F-bar-stabilized deformation gradient. Used by the UL kernel to assemble
  /// the F-bar Jacobian contribution. For F-bar off, _d_F_stab_d_F_ust = I^(4) and
  /// _d_F_stab_d_F_avg = 0, so the kernel chain reduces to the unstabilized case.
  const MaterialProperty<RankFourTensor> & _d_F_stab_d_F_ust;
  const MaterialProperty<RankFourTensor> & _d_F_stab_d_F_avg;

  /// Temperature, if provided.  This is used only to get the trial functions
  const MooseVariable * _temperature;

  /// Out-of-plane strain, if provided.
  const MooseVariable * _out_of_plane_strain;

  /// Eigenstrain derivatives wrt generate coupleds
  std::vector<std::vector<const MaterialProperty<RankTwoTensor> *>> _deigenstrain_dargs;

  /// Derivative of the Cauchy stress with respect to the eigenstrain (published by
  /// ComputeLagrangianObjectiveStress; can be overridden by other Cauchy-providing
  /// materials). Only fetched when eigenstrains are coupled, since only the temperature
  /// off-diagonal Jacobian needs it. Will be nullptr otherwise; subclasses that consume
  /// it must guard accordingly.
  const MaterialProperty<RankFourTensor> * _dcauchy_stress_d_eigenstrain = nullptr;
};
