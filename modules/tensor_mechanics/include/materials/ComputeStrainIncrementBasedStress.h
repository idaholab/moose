//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTESTRAININCREMENTBASEDSTRESS_H
#define COMPUTESTRAININCREMENTBASEDSTRESS_H

#include "ComputeStressBase.h"

class ComputeStrainIncrementBasedStress;

template <>
InputParameters validParams<ComputeStrainIncrementBasedStress>();

/**
 * ComputeStrainIncrementBasedStress computes stress considering list of inelastic strain increments
 */
class ComputeStrainIncrementBasedStress : public ComputeStressBase
{
public:
  ComputeStrainIncrementBasedStress(const InputParameters & parameters);

protected:
  virtual void computeQpStress();
  virtual void computeQpJacobian();

  const MaterialProperty<RankTwoTensor> & _stress_old;
  const MaterialProperty<RankTwoTensor> & _mechanical_strain;
  const MaterialProperty<RankTwoTensor> & _mechanical_strain_old;
  std::vector<const MaterialProperty<RankTwoTensor> *> _inelastic_strains;
  std::vector<const MaterialProperty<RankTwoTensor> *> _inelastic_strains_old;

  std::vector<MaterialPropertyName> _property_names;
  unsigned int _num_property;
};

#endif // COMPUTESTRAININCREMENTBASEDSTRESS_H
