/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTESTRESSFREESTRAINBASE_H
#define COMPUTESTRESSFREESTRAINBASE_H

#include "Material.h"
#include "RankTwoTensor.h"

/**
 * ComputeStressFreeStrainBase is the base class for stress free strain tensors
 */
class ComputeStressFreeStrainBase : public Material
{
public:
  ComputeStressFreeStrainBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
  virtual void computeQpStressFreeStrain() = 0;

  std::string _base_name;

  bool _incremental_form;

  MaterialProperty<RankTwoTensor> & _stress_free_strain;
  MaterialProperty<RankTwoTensor> * _stress_free_strain_old;
  MaterialProperty<RankTwoTensor> & _stress_free_strain_increment;
};

#endif //COMPUTESTRESSFREESTRAINBASE_H
