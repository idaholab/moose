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

  /// Name of the elasticity tensor material property
  const std::string _elasticity_tensor_name;
  /// Elasticity tensor material property
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  /// Old state of the stress tensor material property
  const MaterialProperty<RankTwoTensor> & _stress_old;
  ///@{ Current and old states of the mechanical strain material property
  const MaterialProperty<RankTwoTensor> & _mechanical_strain;
  const MaterialProperty<RankTwoTensor> & _mechanical_strain_old;
  ///@}
  ///@{ Vectors of current and old states of the inelastic strain material properties
  std::vector<const MaterialProperty<RankTwoTensor> *> _inelastic_strains;
  std::vector<const MaterialProperty<RankTwoTensor> *> _inelastic_strains_old;
  ///@}

  /// Names of the inelastic strain material properties for all inelastic models
  std::vector<MaterialPropertyName> _inelastic_strain_names;
  /// Number of inelastic models
  unsigned int _num_inelastic_strain_models;
};

#endif // COMPUTESTRAININCREMENTBASEDSTRESS_H
