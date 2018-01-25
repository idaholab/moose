//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEFINITESTRAIN_H
#define COMPUTEFINITESTRAIN_H

#include "ComputeIncrementalStrainBase.h"

class ComputeFiniteStrain;

template <>
InputParameters validParams<ComputeFiniteStrain>();

/**
 * ComputeFiniteStrain defines a strain increment and rotation increment, for finite strains.
 */
class ComputeFiniteStrain : public ComputeIncrementalStrainBase
{
public:
  ComputeFiniteStrain(const InputParameters & parameters);

  virtual void computeProperties();

  static MooseEnum decompositionType();

protected:
  virtual void computeQpStrain();
  virtual void computeQpIncrements(RankTwoTensor & e, RankTwoTensor & r);

  std::vector<RankTwoTensor> _Fhat;

private:
  enum class DecompMethod
  {
    TaylorExpansion,
    EigenSolution
  };

  const DecompMethod _decomposition_method;
};

#endif // COMPUTEFINITESTRAIN_H
