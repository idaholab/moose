//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LagrangianStressDivergenceBase.h"

/// Enforce equilibrium with an updated Lagrangian formulation
//    This class enforces equilibrium when  used in conjunction with
//    the corresponding strain calculator (CalculateStrainLagrangianKernel)
//    and with either a stress calculator that provides the
//    Cauchy stress ("stress") and the appropriate "cauchy_jacobian",
//    which needs to be the derivative of the increment in Cauchy stress
//    with respect to the increment in the spatial velocity gradient.
//
//    This kernel should be used with the new "ComputeLagrangianStressBase"
//    stress update system and the "ComputeLagrangianStrain" system for strains.
//
//    use_displaced_mesh must be true for large deformation kinematics
//    The kernel enforces this with an error
//
class UpdatedLagrangianStressDivergence : public LagrangianStressDivergenceBase
{
public:
  static InputParameters validParams();
  UpdatedLagrangianStressDivergence(const InputParameters & parameters);

protected:
  /// Implement the residual for some specific _i
  virtual Real computeQpResidual() override;

  /// Implement a specific _i, _j Jacobian
  virtual Real computeQpJacobian() override;
  /// Implement a specific _i, _j Jacobian
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Trial gradient averaging
  virtual void precalculateJacobian() override;

  /// Stabilize a generic gradient tensor
  RankTwoTensor stabilizeGrad(const RankTwoTensor & Gb, const RankTwoTensor & Ga) override;

  /// Calculate the full test gradient (could later be modified for stabilization)
  RankTwoTensor testGrad(unsigned int i);

  /// Calculate the modified trial function gradient for stabilization
  RankTwoTensor trialGrad(unsigned int m, bool stabilize);

private:
  /// A component of the material Jacobian
  Real matJacobianComponent(const RankFourTensor & C,
                            const RankTwoTensor & grad_test,
                            const RankTwoTensor & grad_trial,
                            const RankTwoTensor & df);
  /// A component of the geometric Jacobian
  Real geomJacobianComponent(const RankTwoTensor & grad_test,
                             const RankTwoTensor & grad_trial,
                             const RankTwoTensor & stress);

  /// Compute the average trial function gradient
  void computeAverageGradTrial();

  /// Calculate the average gradient of some type (test or trial)
  void avgGrad(const VariablePhiGradient & grads, std::vector<RealVectorValue> & res);

protected:
  /// Averaged trial function gradients
  std::vector<RealVectorValue> _avg_grad_trial;

  /// The unmodified deformation gradient: needed for mapping averages
  const MaterialProperty<RankTwoTensor> & _uF;

  /// The element-average deformation gradient: needed for mapping averages
  const MaterialProperty<RankTwoTensor> & _aF;

  /// The Cauchy stress
  const MaterialProperty<RankTwoTensor> & _stress;
  /// The derivative of the increment in Cauchy stress wrt the increment in the spatial velocity gradient
  const MaterialProperty<RankFourTensor> & _material_jacobian;
  /// The inverse incremental deformation gradient
  const MaterialProperty<RankTwoTensor> & _df;
};
