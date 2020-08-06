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
 * ComputeCosseratIncrementalSmallStrain defines various incremental versions
 * of the Cossserat strain tensor, assuming small strains.
 */
class ComputeCosseratIncrementalSmallStrain : public ComputeIncrementalStrainBase
{
public:
  static InputParameters validParams();

  ComputeCosseratIncrementalSmallStrain(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  virtual void initQpStatefulProperties();

  /// the Cosserat curvature strain: curvature_ij = nabla_j CosseratRotation_i
  MaterialProperty<RankTwoTensor> & _curvature;

  /// the number of Cosserat rotation variables supplied by the user (must be 3 in current implementation)
  const unsigned int _nrots;

  /// The Cosserat rotations
  const std::vector<const VariableValue *> _wc;

  /// The Cosserat rotations
  const std::vector<const VariableValue *> _wc_old;

  /// Grad(Cosserat rotation)
  const std::vector<const VariableGradient *> _grad_wc;

  /// Grad(Cosserat rotation)
  const std::vector<const VariableGradient *> _grad_wc_old;

  /// the Cosserat curvature strain: curvature_ij = nabla_j CosseratRotation_i
  const MaterialProperty<RankTwoTensor> & _curvature_old;

  /// _curvature_increment = (curvature - _curvature_old)
  MaterialProperty<RankTwoTensor> & _curvature_increment;
};
