//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeIncrementalStrainBase.h"
#include "FactorizedRankTwoTensor.h"

/**
 * ComputeFiniteStrain defines a strain increment and rotation increment, for finite strains.
 */
class ComputeFiniteStrain : public ComputeIncrementalStrainBase
{
public:
  static InputParameters validParams();

  ComputeFiniteStrain(const InputParameters & parameters);

  void computeProperties() override;

  static MooseEnum decompositionType();

protected:
  virtual void computeQpStrain();
  virtual void computeQpIncrements(RankTwoTensor & e, RankTwoTensor & r);

  std::vector<RankTwoTensor> _Fhat;

  enum class DecompMethod
  {
    TaylorExpansion,
    EigenSolution,
    HughesWinget
  };

  const DecompMethod _decomposition_method;

  const bool _use_hw;

  MaterialProperty<RankTwoTensor> * _def_grad_mid;
  MaterialProperty<RankTwoTensor> * _f_bar;
};
