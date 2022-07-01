//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeIncrementalStrainBase.h"
#include "FactorizedRankTwoTensor.h"

#define usingComputeFiniteStrainMembers                                                            \
  usingComputeIncrementalStrainBaseMembers;                                                        \
  using ADComputeFiniteStrainTempl<R2, R4>::_Fhat

/**
 * ADComputeFiniteStrain defines a strain increment and rotation increment, for finite strains.
 */
template <typename R2, typename R4>
class ADComputeFiniteStrainTempl : public ADComputeIncrementalStrainBaseTempl<R2>
{
public:
  static InputParameters validParams();

  ADComputeFiniteStrainTempl(const InputParameters & parameters);

  void computeProperties() override;

  static MooseEnum decompositionType();

protected:
  using ADR2 = Moose::GenericType<R2, true>;
  using FADR2 = FactorizedRankTwoTensorTempl<ADR2>;

  virtual void computeQpStrain();
  virtual void computeQpIncrements(ADR2 & e, ADRankTwoTensor & r);

  std::vector<ADRankTwoTensor> _Fhat;

  enum class DecompMethod
  {
    TaylorExpansion,
    EigenSolution
  };
  const DecompMethod _decomposition_method;

  usingComputeIncrementalStrainBaseMembers;
};

typedef ADComputeFiniteStrainTempl<RankTwoTensor, RankFourTensor> ADComputeFiniteStrain;
typedef ADComputeFiniteStrainTempl<SymmetricRankTwoTensor, SymmetricRankFourTensor>
    ADSymmetricFiniteStrain;
