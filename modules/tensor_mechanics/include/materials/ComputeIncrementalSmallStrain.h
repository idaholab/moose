/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEINCREMENTALSMALLSTRAIN_H
#define COMPUTEINCREMENTALSMALLSTRAIN_H

#include "ComputeIncrementalStrainBase.h"

/**
 * ComputeIncrementalSmallStrain defines a strain increment and rotation increment (=1), for small
 * strains.
 */
class ComputeIncrementalSmallStrain : public ComputeIncrementalStrainBase
{
public:
  ComputeIncrementalSmallStrain(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;

  /// Computes the current and old deformation gradients and passes back the
  /// total strain increment tensor
  virtual void computeTotalStrainIncrement(RankTwoTensor & total_strain_increment);
};

#endif // COMPUTEINCREMENTALSMALLSTRAIN_H
