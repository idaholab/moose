//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementOptimizationVJPInnerProduct.h"

registerMooseObject("OptimizationApp", ElementOptimizationVJPInnerProduct);

InputParameters
ElementOptimizationVJPInnerProduct::validParams()
{
  InputParameters params = ElementOptimizationFunctionInnerProduct::validParams();
  params.addClassDescription(
      "Computes the gradient of the objective function with respect to a NEML2 parameter by "
      "integrating the scalar vector-Jacobian product material property emitted by the NEML2 "
      "executor.");
  params.addRequiredParam<MaterialPropertyName>(
      "vjp_name",
      "The name of the scalar vector-Jacobian product material property declared by the NEML2 "
      "action, named 'vjp_<neml2_output>_<parameter>', e.g. 'vjp_neml2_stress_elasticity_E'.");
  return params;
}

ElementOptimizationVJPInnerProduct::ElementOptimizationVJPInnerProduct(
    const InputParameters & parameters)
  : ElementOptimizationFunctionInnerProduct(parameters),
    _vjp(getMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("vjp_name")))
{
}

Real
ElementOptimizationVJPInnerProduct::computeQpInnerProduct()
{
  return -_vjp[_qp];
}
