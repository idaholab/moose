//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEINCREMENTALSMALLSTRAIN_H
#define COMPUTEINCREMENTALSMALLSTRAIN_H

#include "ComputeIncrementalStrainBase.h"

class ComputeIncrementalSmallStrain;

template <>
InputParameters validParams<ComputeIncrementalSmallStrain>();

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
