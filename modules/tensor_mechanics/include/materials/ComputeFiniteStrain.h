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

  virtual void computeProperties();

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpStrain();
  virtual void computeQpIncrements(RankTwoTensor & e, RankTwoTensor & r);

  MaterialProperty<RankTwoTensor> & _strain_rate;
  MaterialProperty<RankTwoTensor> & _strain_increment;
  MaterialProperty<RankTwoTensor> & _mechanical_strain_old;
  MaterialProperty<RankTwoTensor> & _total_strain_old;
  MaterialProperty<RankTwoTensor> & _rotation_increment;

  MaterialProperty<RankTwoTensor> & _deformation_gradient;
  MaterialProperty<RankTwoTensor> & _deformation_gradient_old;

  const MaterialProperty<RankTwoTensor> & _eigenstrain_increment;

  const Real & _current_elem_volume;
  std::vector<RankTwoTensor> _Fhat;

private:
  enum class DecompMethod
  {
    TaylorExpansion,
    EigenSolution
  };

  const DecompMethod _decomposition_method;
};

#endif //COMPUTEFINITESTRAIN_H
