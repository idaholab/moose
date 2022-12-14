//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideOptimizationFunctionInnerProduct.h"

InputParameters
SideOptimizationFunctionInnerProduct::validParams()
{
  InputParameters params = SideVectorPostprocessor::validParams();
  params += OptimizationFunctionInnerProductHelper::validParams();
  params.addRequiredCoupledVar(
      "variable", "Variable used for inner product calculation, usually the adjoint variable.");
  return params;
}

SideOptimizationFunctionInnerProduct::SideOptimizationFunctionInnerProduct(
    const InputParameters & parameters)
  : SideVectorPostprocessor(parameters),
    OptimizationFunctionInnerProductHelper(parameters),
    _var(coupledValue("variable")),
    _vec(declareVector("inner_product"))
{
}

void
SideOptimizationFunctionInnerProduct::initialize()
{
  setCurrentTime(_t, _dt);
}

void
SideOptimizationFunctionInnerProduct::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    const Real q_inner_product = _JxW[_qp] * _coord[_qp] * computeQpInnerProduct();
    update(_q_point[_qp], q_inner_product);
  }
}

void
SideOptimizationFunctionInnerProduct::threadJoin(const UserObject & y)
{
  const auto & vpp = static_cast<const SideOptimizationFunctionInnerProduct &>(y);
  add(vpp);
}

void
SideOptimizationFunctionInnerProduct::finalize()
{
  getVector(_vec);
}
