//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BatchPropertyDerivative.h"
#include "ElementOptimizationFunctionInnerProduct.h"
#include "SymmetricRankTwoTensor.h"

/**
 * This is a "glue" material that retrieves the batched stress derivative output vector
 * and uses the output variables to perform the objective stress integration.
 */
class AdjointStrainStressGradInnerProduct : public ElementOptimizationFunctionInnerProduct
{
public:
  static InputParameters validParams();
  AdjointStrainStressGradInnerProduct(const InputParameters & parameters);

protected:
  virtual Real computeQpInnerProduct() override;

  /// Base name of the material system
  const std::string _base_name;

  /// Holds stress derivative at current quadrature points
  const MaterialProperty<SymmetricRankTwoTensor> & _stress_derivative;

  /// Holds adjoint strain at current quadrature points
  const MaterialProperty<RankTwoTensor> & _adjoint_strain;
};
