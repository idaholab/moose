//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementOptimizationFunctionInnerProduct.h"
#include "SymmetricRankTwoTensor.h"

/**
 * This object calculates the double contraction between the stress derivative and the adjoint
 * strain material properties. The resulting product is integrated to determine the gradient of the
 * objective function, which is essential for solving an inverse optimization problem.
 */
class AdjointStrainSymmetricStressGradInnerProduct : public ElementOptimizationFunctionInnerProduct
{
public:
  static InputParameters validParams();
  AdjointStrainSymmetricStressGradInnerProduct(const InputParameters & parameters);

protected:
  virtual Real computeQpInnerProduct() override;

  /// Base name of the material system
  const std::string _base_name;

  /// Holds stress derivative at current quadrature points
  const MaterialProperty<SymmetricRankTwoTensor> & _stress_derivative;

  /// Holds adjoint strain at current quadrature points
  const MaterialProperty<RankTwoTensor> & _adjoint_strain;
};
