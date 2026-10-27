//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStressBase.h"

/// Native interface for providing the Cauchy stress
///
/// This class *implements* the Cauchy stress update, providing
///   1) the Cauchy stress
///   2) the derivative of the increment in Cauchy stress wrt the
///      increment in the spatial velocity gradient
///
/// And wraps these quantities to provide
///   1) the 1st PK stress
///   2) the derivative of the 1st PK stress wrt the deformation gradient
///
class ComputeLagrangianStressCauchy : public ComputeLagrangianStressBase
{
public:
  static InputParameters validParams();
  ComputeLagrangianStressCauchy(const InputParameters & parameters);

protected:
  /// Calculate the stress update to provide both measures (cauchy and pk)
  virtual void computeQpStressUpdate() override;
  /// Provide for the actual Cauchy stress update (just cauchy)
  virtual void computeQpCauchyStress() = 0;

private:
  /// Wrap the Cauchy stress to get the PK stress
  virtual void computeQpPK1Stress();

protected:
  /// Inverse incremental deformation gradient
  const MaterialProperty<RankTwoTensor> & _inv_df;
  /// Inverse F-bar-stabilized deformation gradient (= _F^{-1}). Kept for any internal
  /// consumer that still needs it; the kinematic stress-measure wrap uses _F_ust below.
  const MaterialProperty<RankTwoTensor> & _inv_def_grad;
  /// F-bar-stabilized deformation gradient (= the strain calc's published `_F`). Drives
  /// the constitutive update via the strain calc's `_f_inv` chain; NOT used for the
  /// kinematic Cauchy -> PK1 wrap (that's `_F_ust`, declared on the base).
  const MaterialProperty<RankTwoTensor> & _F;
  /// Inverse and determinant of the unstabilized deformation gradient, published by the strain
  /// calculator (computed once per qp there). Consumed for the Cauchy -> PK1 wrap instead of
  /// recomputing `_F_ust.inverse()`/`.det()` per qp on every residual/Jacobian sweep.
  const MaterialProperty<RankTwoTensor> & _F_ust_inv;
  const MaterialProperty<Real> & _F_ust_det;
};
