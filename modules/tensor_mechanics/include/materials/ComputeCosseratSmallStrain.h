//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTECOSSERATSMALLSTRAIN_H
#define COMPUTECOSSERATSMALLSTRAIN_H

#include "ComputeStrainBase.h"

class ComputeCosseratSmallStrain;

template <>
InputParameters validParams<ComputeCosseratSmallStrain>();

/**
 * ComputeCosseratSmallStrain defines Cossserat strain tensor, assuming small strains.
 */
class ComputeCosseratSmallStrain : public ComputeStrainBase
{
public:
  ComputeCosseratSmallStrain(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// the Cosserat curvature strain: curvature_ij = nabla_j CosseratRotation_i
  MaterialProperty<RankTwoTensor> & _curvature;

  /// the number of Cosserat rotation variables supplied by the user (must be 3 in current implementation)
  const unsigned int _nrots;

  /// The Cosserat rotations
  std::vector<const VariableValue *> _wc;

  /// Grad(Cosserat rotation)
  std::vector<const VariableGradient *> _grad_wc;
};

#endif // COMPUTECOSSERATSMALLSTRAIN_H
