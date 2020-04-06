//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensorMechanicsPlasticModel.h"

/**
 * Class that can be used for testing multi-surface plasticity models.
 * Yield function = a*stress_yy + b*stress_zz + c*stress_xx + d*(stress_xy + stress_yx)/2 +
 * e*(stress_xz + stress_zx)/2 + f*(stress_yz + stress_zy)/2 - strength
 * No hardening/softening.  Associative.
 */
class TensorMechanicsPlasticSimpleTester : public TensorMechanicsPlasticModel
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticSimpleTester(const InputParameters & parameters);

  virtual std::string modelName() const override;

protected:
  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const override;

  Real dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const override;

  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  /// a
  Real _a;

  /// b
  Real _b;

  /// c
  Real _c;

  /// d
  Real _d;

  /// e
  Real _e;

  /// f
  Real _f;

  /// strength
  Real _strength;
};
