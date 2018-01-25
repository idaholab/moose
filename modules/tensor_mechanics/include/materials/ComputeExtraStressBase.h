//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEEXTRASTRESSBASE_H
#define COMPUTEEXTRASTRESSBASE_H

#include "Material.h"
#include "RankTwoTensor.h"

class ComputeExtraStressBase;

template <>
InputParameters validParams<ComputeExtraStressBase>();

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
