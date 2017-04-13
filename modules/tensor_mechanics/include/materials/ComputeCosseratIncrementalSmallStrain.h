/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECOSSERATINCREMENTALSMALLSTRAIN_H
#define COMPUTECOSSERATINCREMENTALSMALLSTRAIN_H

#include "ComputeIncrementalStrainBase.h"

/**
 * ComputeCosseratIncrementalSmallStrain defines various incremental versions
 * of the Cossserat strain tensor, assuming small strains.
 */
class ComputeCosseratIncrementalSmallStrain : public ComputeIncrementalStrainBase
{
public:
  ComputeCosseratIncrementalSmallStrain(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  virtual void initQpStatefulProperties();

  /// the Cosserat curvature strain: curvature_ij = nabla_j CosseratRotation_i
  MaterialProperty<RankTwoTensor> & _curvature;

  /// the number of Cosserat rotation variables supplied by the user (must be 3 in current implementation)
  const unsigned int _nrots;

  /// The Cosserat rotations
  std::vector<const VariableValue *> _wc;

  /// The Cosserat rotations
  std::vector<const VariableValue *> _wc_old;

  /// Grad(Cosserat rotation)
  std::vector<const VariableGradient *> _grad_wc;

  /// Grad(Cosserat rotation)
  std::vector<const VariableGradient *> _grad_wc_old;

  /// the Cosserat curvature strain: curvature_ij = nabla_j CosseratRotation_i
  MaterialProperty<RankTwoTensor> & _curvature_old;

  /// _curvature_increment = (curvature - _curvature_old)
  MaterialProperty<RankTwoTensor> & _curvature_increment;
};

#endif // COMPUTECOSSERATINCREMENTALSMALLSTRAIN_H
