//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianObjectiveStress.h"

/// Use  MOOSE materials deriving from ComputeStressBase with Lagrangian kernels
///
/// This class wraps existing MOOSE materials inheriting from ComputeStressBase
/// from use with the Lagrangian kernel system.
/// The wrapper only uses the *small deformation* form of the material model.
/// If the kernel requires a large deformation stress update then this wrapper
/// supplies that by applying the ComputeLagrangianObjectiveStress object
/// rotation.
///
/// This means that ComputeStressBase materials used in finite deformation
/// calculations will produce somewhat different results when comparing
/// the StressDivergenceTensor updated Lagrangian kernel to either the
/// TotalLagrangian or UpdatedLagrangian kernels.
///
class ComputeLagrangianWrappedStress : public ComputeLagrangianObjectiveStress
{
public:
  static InputParameters validParams();
  ComputeLagrangianWrappedStress(const InputParameters & parameters);

protected:
  /// Wraps the stress update
  virtual void computeQpSmallStress() override;

protected:
  /// The input small stress
  const MaterialProperty<RankTwoTensor> & _input_stress;
  /// The input Jacobian
  const MaterialProperty<RankFourTensor> & _input_jacobian;
};
