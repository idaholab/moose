/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEFINITESTRAIN_H
#define COMPUTEFINITESTRAIN_H

#include "ComputeStrainBase.h"

/**
 * ComputeFiniteStrain defines a strain increment and rotation increment, for finite strains.
 */
class ComputeFiniteStrain : public ComputeStrainBase
{
public:
  ComputeFiniteStrain(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeProperties();
  virtual void computeQpStrain(const RankTwoTensor & Fhat);

  MaterialProperty<RankTwoTensor> & _strain_rate;
  MaterialProperty<RankTwoTensor> & _strain_increment;
  MaterialProperty<RankTwoTensor> & _mechanical_strain_old;
  MaterialProperty<RankTwoTensor> & _total_strain_old;
  MaterialProperty<RankTwoTensor> & _rotation_increment;

  MaterialProperty<RankTwoTensor> & _deformation_gradient;
  MaterialProperty<RankTwoTensor> & _deformation_gradient_old;

  const MaterialProperty<RankTwoTensor> & _stress_free_strain_increment;
  const VariableValue & _T_old;
};

#endif //COMPUTEFINITESTRAIN_H
