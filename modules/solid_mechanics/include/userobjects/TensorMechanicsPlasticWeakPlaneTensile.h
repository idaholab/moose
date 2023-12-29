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
#include "TensorMechanicsHardeningModel.h"

/**
 * Rate-independent associative weak-plane tensile failure
 * with hardening/softening of the tensile strength
 */
class TensorMechanicsPlasticWeakPlaneTensile : public TensorMechanicsPlasticModel
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticWeakPlaneTensile(const InputParameters & parameters);

  virtual void activeConstraints(const std::vector<Real> & f,
                                 const RankTwoTensor & stress,
                                 Real intnl,
                                 const RankFourTensor & Eijkl,
                                 std::vector<bool> & act,
                                 RankTwoTensor & returned_stress) const override;

  virtual std::string modelName() const override;

protected:
  /// Yield function = _a * stress_zz - _strength;
  const Real _a;

  /// Yield function = _a * stress_zz - _strength;
  const TensorMechanicsHardeningModel & _strength;

  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const override;

  Real dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const override;

  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  /// tensile strength as a function of residual value, rate, and internal_param
  virtual Real tensile_strength(const Real internal_param) const;

  /// d(tensile strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dtensile_strength(const Real internal_param) const;
};
