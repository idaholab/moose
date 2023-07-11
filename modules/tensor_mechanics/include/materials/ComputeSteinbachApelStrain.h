//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialInterface.h"

class ComputeSteinbachApelStrain : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ComputeSteinbachApelStrain(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;  

  const std::string _base_name;

  const MaterialProperty<Real> & _h_eta;

  std::vector<const MaterialProperty<Real> *> _dh;

  std::string _base_A;
  std::string _base_B;

  const std::string _elasticity_tensor_B_name;
  const MaterialProperty<RankFourTensor> & _elasticity_tensor_B;
  
  const std::string _compliance_tensor_A_name;
  const MaterialProperty<RankFourTensor> & _compliance_tensor_A;

  const std::string _global_strain_name;
  const MaterialProperty<RankTwoTensor> & _global_mechanical_strain; 

  MaterialProperty<RankTwoTensor> & _mechanical_strain_a;
  MaterialProperty<RankTwoTensor> & _mechanical_strain_b;

  MaterialProperty<RankFourTensor> & _ref;
  MaterialProperty<RankFourTensor> & _ref_inv;

  MaterialProperty<RankFourTensor> & _identity;

  float _identity_two[6][6];
};
