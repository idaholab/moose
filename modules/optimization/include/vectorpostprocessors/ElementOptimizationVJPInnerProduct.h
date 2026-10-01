//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementOptimizationFunctionInnerProduct.h"

/**
 * This object integrates a scalar vector-Jacobian product, computed by the NEML2 executor as the
 * contraction of the adjoint strain with the derivative of the stress with respect to a NEML2
 * parameter, against the parameter basis. The resulting product determines the gradient of the
 * objective function, which is essential for solving an inverse optimization problem.
 */
class ElementOptimizationVJPInnerProduct : public ElementOptimizationFunctionInnerProduct
{
public:
  static InputParameters validParams();
  ElementOptimizationVJPInnerProduct(const InputParameters & parameters);

protected:
  virtual Real computeQpInnerProduct() override;

  /// Holds the vector-Jacobian product at current quadrature points
  const MaterialProperty<Real> & _vjp;
};
