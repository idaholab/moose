/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEINCREMENTALSMALLSTRAIN_H
#define COMPUTEINCREMENTALSMALLSTRAIN_H

#include "ComputeSmallStrain.h"

/**
 * ComputeIncrementalSmallStrain defines a strain increment and rotation increment (=1), for small strains.
 */
class ComputeIncrementalSmallStrain : public ComputeSmallStrain
{
public:
  ComputeIncrementalSmallStrain(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeProperties();

  /// Computes the current and old deformation gradients and passes back the
  /// total strain increment tensor
  virtual void computeTotalStrainIncrement(RankTwoTensor & total_strain_increment);

  MaterialProperty<RankTwoTensor> & _strain_rate;
  MaterialProperty<RankTwoTensor> & _strain_increment;
  MaterialProperty<RankTwoTensor> & _mechanical_strain_old;
  MaterialProperty<RankTwoTensor> & _total_strain_old;
  MaterialProperty<RankTwoTensor> & _rotation_increment;

  MaterialProperty<RankTwoTensor> & _deformation_gradient;

  const MaterialProperty<RankTwoTensor> & _stress_free_strain_increment;
};

#endif //COMPUTEINCREMENTALSMALLSTRAIN_H
