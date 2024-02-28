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

/**
 * ComputeIncrementalSmallStrain defines a strain increment and rotation increment (=1), for small
 * strains.
 */
class ComputeIncrementalSmallStrain : public ComputeIncrementalStrainBase
{
public:
  static InputParameters validParams();

  ComputeIncrementalSmallStrain(const InputParameters & parameters);

  virtual void computeProperties() override;

protected:
  /// Computes the current and old deformation gradients and passes back the
  /// total strain increment tensor
  virtual void computeTotalStrainIncrement(RankTwoTensor & total_strain_increment);
};
