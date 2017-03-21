/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTESTRAININCREMENTBASEDSTRESS_H
#define COMPUTESTRAININCREMENTBASEDSTRESS_H

#include "ComputeStressBase.h"

class ComputeStrainIncrementBasedStress;

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

  MaterialProperty<RankTwoTensor> & _stress_old;
  const MaterialProperty<RankTwoTensor> & _mechanical_strain;
  const MaterialProperty<RankTwoTensor> & _mechanical_strain_old;
  std::vector<const MaterialProperty<RankTwoTensor> *> _inelastic_strains;
  std::vector<const MaterialProperty<RankTwoTensor> *> _inelastic_strains_old;

  std::vector<MaterialPropertyName> _property_names;
  unsigned int _num_property;
};

#endif // COMPUTESTRAININCREMENTBASEDSTRESS_H
