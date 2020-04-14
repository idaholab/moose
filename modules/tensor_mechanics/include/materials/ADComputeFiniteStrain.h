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

/**
 * ADComputeFiniteStrain defines a strain increment and rotation increment, for finite strains.
 */
class ADComputeFiniteStrain : public ADComputeIncrementalStrainBase
{
public:
  static InputParameters validParams();

  ADComputeFiniteStrain(const InputParameters & parameters);

  void computeProperties() override;

  static MooseEnum decompositionType();

protected:
  virtual void computeQpStrain();
  virtual void computeQpIncrements(ADRankTwoTensor & e, ADRankTwoTensor & r);

  std::vector<ADRankTwoTensor> _Fhat;

private:
  enum class DecompMethod
  {
    TaylorExpansion,
    EigenSolution
  };

  const DecompMethod _decomposition_method;

protected:
};
