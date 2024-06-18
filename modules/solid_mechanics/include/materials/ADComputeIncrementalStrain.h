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

#define usingComputeIncrementalStrainMembers usingComputeIncrementalStrainBaseMembers

/**
 * ADComputeIncrementalStrainTempl defines a strain increment and rotation increment (=1), for
 * small strains.
 */
template <typename R2>
class ADComputeIncrementalStrainTempl : public ADComputeIncrementalStrainBaseTempl<R2>
{
public:
  static InputParameters validParams();

  ADComputeIncrementalStrainTempl(const InputParameters & parameters);

  virtual void computeProperties() override;

protected:
  using ADR2 = Moose::GenericType<R2, true>;

  /**
   * Computes the current and old deformation gradients and passes back the
   * total strain increment tensor.
   */
  virtual void computeTotalStrainIncrement(ADR2 & total_strain_increment);

  usingComputeIncrementalStrainBaseMembers;
};

typedef ADComputeIncrementalStrainTempl<RankTwoTensor> ADComputeIncrementalStrain;
typedef ADComputeIncrementalStrainTempl<SymmetricRankTwoTensor> ADSymmetricIncrementalSmallStrain;
