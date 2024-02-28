//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  /// Inverse deformation gradient
  const MaterialProperty<RankTwoTensor> & _inv_def_grad;
  /// Deformation gradient
  const MaterialProperty<RankTwoTensor> & _F;
};
