/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEEXTRASTRESSBASE_H
#define COMPUTEEXTRASTRESSBASE_H

#include "Material.h"
#include "RankTwoTensor.h"

/**
 * ComputeExtraStressBase is the base class for extra_stress, which is added to stress
 * calculated by the material's constitutive model
 */
class ComputeExtraStressBase : public Material
{
public:
  ComputeExtraStressBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void computeQpExtraStress() = 0;

  std::string _base_name;
  std::string _extra_stress_name;

  MaterialProperty<RankTwoTensor> & _extra_stress;
};

#endif // COMPUTEEXTRASTRESSBASE_H
