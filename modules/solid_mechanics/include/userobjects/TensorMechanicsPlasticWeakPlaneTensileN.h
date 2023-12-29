//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensorMechanicsPlasticWeakPlaneTensile.h"

/**
 * Rate-independent associative weak-plane tensile failure
 * with hardening/softening, and normal direction specified
 */
class TensorMechanicsPlasticWeakPlaneTensileN : public TensorMechanicsPlasticWeakPlaneTensile
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticWeakPlaneTensileN(const InputParameters & parameters);

  virtual std::string modelName() const override;

protected:
  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const override;

  Real dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const override;

  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  /// Unit normal inputted by user
  RealVectorValue _input_n;

  /// Flow direction, which is constant in this case
  RankTwoTensor _df_dsig;

  /// This rotation matrix rotates _input_n to (0, 0, 1)
  RealTensorValue _rot;
};
