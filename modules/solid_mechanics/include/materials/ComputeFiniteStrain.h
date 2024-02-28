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

  enum class DecompMethod
  {
    TaylorExpansion,
    EigenSolution,
    HughesWinget
  };

  static MooseEnum decompositionType();

protected:
  virtual void computeQpStrain();
  virtual void computeQpIncrements(RankTwoTensor & e, RankTwoTensor & r);

  /// Incremental deformation gradient
  std::vector<RankTwoTensor> _Fhat;

  /// Method for determining rotation and strain increments
  const DecompMethod _decomposition_method;

  /// Flag if using HughesWinget method
  const bool _use_hw;

  /// For HughesWinget kinematics
  MaterialProperty<RankTwoTensor> * _def_grad_mid;
  MaterialProperty<RankTwoTensor> * _f_bar;
};
