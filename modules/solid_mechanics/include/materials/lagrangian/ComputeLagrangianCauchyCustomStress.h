//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStressCauchy.h"
#include "DerivativeMaterialPropertyNameInterface.h"

/// Provide the Cauchy stress and jacobian directly
///
class ComputeLagrangianCauchyCustomStress : public ComputeLagrangianStressCauchy,
                                            public DerivativeMaterialPropertyNameInterface
{
public:
  static InputParameters validParams();
  ComputeLagrangianCauchyCustomStress(const InputParameters & parameters);

protected:
  /// Implement the copy
  virtual void computeQpCauchyStress() override;

  const MaterialProperty<RankTwoTensor> & _custom_stress;
  const MaterialProperty<RankFourTensor> & _custom_jacobian;
};
