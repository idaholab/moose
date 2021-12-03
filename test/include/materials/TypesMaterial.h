//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DenseMatrix.h"
#include "Material.h"
#include "RankTwoTensorForward.h"
#include "RankThreeTensorForward.h"
#include "RankFourTensorForward.h"

/**
 * Material for testing different types of material properties
 */
class TypesMaterial : public Material
{
public:
  static InputParameters validParams();

  TypesMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  MaterialProperty<Real> & _real_prop;
  MaterialProperty<std::vector<Real>> & _std_vec_prop;
  const Real _std_vec_prop_entry1;
  MaterialProperty<std::vector<Real>> & _std_vec_prop_qp;
  MaterialProperty<std::vector<RealGradient>> & _std_vec_grad_prop;
  MaterialProperty<RealVectorValue> & _real_vec_prop;
  MaterialProperty<RealGradient> & _real_gradient_prop;
  MaterialProperty<DenseMatrix<Real>> & _matrix_prop;
  MaterialProperty<RealTensorValue> & _tensor_prop;
  MaterialProperty<RankTwoTensor> & _rank_two_tensor_prop;
  MaterialProperty<RankThreeTensor> & _rank_three_tensor_prop;
  MaterialProperty<RankFourTensor> & _rank_four_tensor_prop;
};
