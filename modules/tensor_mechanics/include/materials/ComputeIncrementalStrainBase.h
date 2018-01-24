/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEINCREMENTALSTRAINBASE_H
#define COMPUTEINCREMENTALSTRAINBASE_H

#include "ComputeStrainBase.h"

/**
 * ComputeIncrementalStrainBase is the base class for strain tensors using incremental formulations
 */
class ComputeIncrementalStrainBase : public ComputeStrainBase
{
public:
  ComputeIncrementalStrainBase(const InputParameters & parameters);
  virtual ~ComputeIncrementalStrainBase() {}

protected:
  virtual void initQpStatefulProperties() override;

  void subtractEigenstrainIncrementFromStrain(RankTwoTensor & strain);

  std::vector<const VariableGradient *> _grad_disp_old;

  MaterialProperty<RankTwoTensor> & _strain_rate;
  MaterialProperty<RankTwoTensor> & _strain_increment;
  MaterialProperty<RankTwoTensor> & _rotation_increment;

  MaterialProperty<RankTwoTensor> & _deformation_gradient;

  const MaterialProperty<RankTwoTensor> & _mechanical_strain_old;
  const MaterialProperty<RankTwoTensor> & _total_strain_old;

  std::vector<const MaterialProperty<RankTwoTensor> *> _eigenstrains_old;
};

#endif // COMPUTEINCREMENTALSTRAINBASE_H
