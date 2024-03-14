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

/**
 * This is a "glue" material that retrieves the batched stress derivative output vector
 * and uses the output variables to perform the objective stress integration.
 */
class AdjointStrainBatchStressGradInnerProduct : public ElementOptimizationFunctionInnerProduct
{
public:
  static InputParameters validParams();
  AdjointStrainBatchStressGradInnerProduct(const InputParameters & parameters);

protected:
  virtual Real computeQpInnerProduct() override;

  /// Base name of the material system
  const std::string _base_name;
  /// Holds adjoint strain at current quadrature points
  const MaterialProperty<RankTwoTensor> & _adjoint_strain;

  /// The userobject that contains the property derivative
  const BatchPropertyDerivativeRankTwoTensorReal & _derivative_uo;

  /// The output from the NEML2 userobject
  const BatchPropertyDerivativeRankTwoTensorReal::OutputVector & _derivative;
};
