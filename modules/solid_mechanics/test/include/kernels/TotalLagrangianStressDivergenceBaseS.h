//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LagrangianStressDivergenceBaseS.h"
#include "GradientOperator.h"

/// Enforce equilibrium with a total Lagrangian formulation
///
/// This class enforces equilibrium when used in conjunction with
/// the corresponding strain calculator (CalculateStrainLagrangianKernel)
/// and with either a stress calculator that provides the
/// 1st PK stress ("pk1_stress") and the derivative of the 1st PK stress
/// with respect to the deformation gradient ("pk1_jacobian")
///
/// This kernel should be used with the new "ComputeLagrangianStressBase"
/// stress update system and the "ComputeLagrangianStrain" system for strains.
///
template <class G>
class TotalLagrangianStressDivergenceBaseS : public LagrangianStressDivergenceBaseS, G
{
public:
  static InputParameters baseParams();
  static InputParameters validParams();
  TotalLagrangianStressDivergenceBaseS(const InputParameters & parameters);
  virtual void initialSetup() override;

protected:
  virtual RankTwoTensor gradTest(unsigned int component) override;
  virtual RankTwoTensor gradTrial(unsigned int component) override;
  virtual void precalculateJacobianDisplacement(unsigned int component) override;
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobianDisplacement(unsigned int alpha, unsigned int beta) override;
  virtual Real computeQpJacobianTemperature(unsigned int cvar) override;
  virtual Real computeQpJacobianOutOfPlaneStrain() override;

  /// The 1st Piola-Kirchhoff stress
  const MaterialProperty<RankTwoTensor> & _pk1;

  /// The derivative of the PK1 stress with respect to the
  /// deformation gradient
  const MaterialProperty<RankFourTensor> & _dpk1;

private:
  /// The unstabilized trial function gradient
  virtual RankTwoTensor gradTrialUnstabilized(unsigned int component);

  /// The stabilized trial function gradient
  virtual RankTwoTensor gradTrialStabilized(unsigned int component);
};
