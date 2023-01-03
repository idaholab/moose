//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementOptimizationFunctionInnerProduct.h"

InputParameters
ElementOptimizationFunctionInnerProduct::validParams()
{
  InputParameters params = ElementVariableVectorPostprocessor::validParams();
  params += OptimizationFunctionInnerProductHelper::validParams();
  params.addClassDescription("Computes the inner product of variable with parameterized source "
                             "function for optimization gradient computation.");
  return params;
}

ElementOptimizationFunctionInnerProduct::ElementOptimizationFunctionInnerProduct(
    const InputParameters & parameters)
  : ElementVariableVectorPostprocessor(parameters),
    OptimizationFunctionInnerProductHelper(parameters),
    _var(coupledValue("variable")),
    _vec(declareVector("inner_product"))
{
}

void
ElementOptimizationFunctionInnerProduct::initialize()
{
  setCurrentTime(_t, _dt);
}

void
ElementOptimizationFunctionInnerProduct::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    const Real q_inner_product = _JxW[_qp] * _coord[_qp] * computeQpInnerProduct();
    update(_q_point[_qp], q_inner_product);
  }
}

void
ElementOptimizationFunctionInnerProduct::threadJoin(const UserObject & y)
{
  const auto & vpp = static_cast<const ElementOptimizationFunctionInnerProduct &>(y);
  add(vpp);
}

void
ElementOptimizationFunctionInnerProduct::finalize()
{
  getVector(_vec);
}
