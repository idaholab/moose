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
#include "GradientOperator.h"
#include "Assembly.h"

/// Enforce equilibrium with an updated Lagrangian formulation
///
/// This class enforces equilibrium when  used in conjunction with
/// the corresponding strain calculator (CalculateStrainLagrangianKernel)
/// and with either a stress calculator that provides the
/// Cauchy stress ("stress") and the appropriate "cauchy_jacobian",
/// which needs to be the derivative of the increment in Cauchy stress
/// with respect to the increment in the spatial velocity gradient.
///
/// This kernel should be used with the new "ComputeLagrangianStressBase"
/// stress update system and the "ComputeLagrangianStrain" system for strains.
///
/// use_displaced_mesh must be true for large deformation kinematics
/// The kernel enforces this with an error
///
template <class G>
class UpdatedLagrangianStressDivergenceBase : public LagrangianStressDivergenceBase, public G
{
public:
  static InputParameters baseParams()
  {
    InputParameters params = LagrangianStressDivergenceBase::validParams();
    return params;
  }
  static InputParameters validParams();
  UpdatedLagrangianStressDivergenceBase(const InputParameters & parameters);
  virtual void initialSetup() override;

protected:
  virtual RankTwoTensor gradTest(unsigned int component) override;
  virtual RankTwoTensor gradTrial(unsigned int component) override;
  virtual void precalculateJacobianDisplacement(unsigned int component) override;
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobianDisplacement(unsigned int alpha, unsigned int beta) override;
  virtual Real computeQpJacobianTemperature(unsigned int cvar) override;
  virtual Real computeQpJacobianOutOfPlaneStrain() override { return 0; }

  /// The Cauchy stress
  const MaterialProperty<RankTwoTensor> & _stress;

  // The derivative of the increment in Cauchy stress w.r.t. the increment in the spatial velocity
  // gradient
  const MaterialProperty<RankFourTensor> & _material_jacobian;

  // @{
  // The assembly quantities in the reference frame for stabilization
  Assembly & _assembly_undisplaced;
  const VariablePhiGradient & _grad_phi_undisplaced;
  const MooseArray<Real> & _JxW_undisplaced;
  const MooseArray<Real> & _coord_undisplaced;
  const MooseArray<Point> & _q_point_undisplaced;
  // @}

private:
  /// The unstabilized trial function gradient
  virtual RankTwoTensor gradTrialUnstabilized(unsigned int component);

  /// The stabilized trial function gradient
  virtual RankTwoTensor gradTrialStabilized(unsigned int component);
};

template <>
inline InputParameters
UpdatedLagrangianStressDivergenceBase<GradientOperatorCartesian>::validParams()
{
  InputParameters params = UpdatedLagrangianStressDivergenceBase::baseParams();
  params.addClassDescription(
      "Enforce equilibrium with an updated Lagrangian formulation in Cartesian coordinates.");
  return params;
}

template <>
inline void
UpdatedLagrangianStressDivergenceBase<GradientOperatorCartesian>::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_XYZ)
    mooseError("This kernel should only act in Cartesian coordinates.");
}

typedef UpdatedLagrangianStressDivergenceBase<GradientOperatorCartesian>
    UpdatedLagrangianStressDivergence;
