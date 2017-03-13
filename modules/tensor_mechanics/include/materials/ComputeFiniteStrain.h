/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEFINITESTRAIN_H
#define COMPUTEFINITESTRAIN_H

#include "ComputeIncrementalStrainBase.h"

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
