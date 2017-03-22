/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "GrainBoundaryArea.h"
#include "MooseUtils.h"

template <>
InputParameters
validParams<GrainBoundaryArea>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addClassDescription("Calculate total grain boundary length in 2D and area in 3D");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  params.addParam<Real>("grains_per_side",
                        2.0,
                        "Number of order parameters contacting a boundary "
                        "(should be 2.0 in polycrystals and 1.0 for "
                        "dispersed particles)");
  params.addParam<Real>("op_range",
                        1.0,
                        "Range over which order parameters change across an "
                        "interface. By default order parameters are assumed to "
                        "vary from 0 to 1");
  return params;
}

GrainBoundaryArea::GrainBoundaryArea(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _op_num(coupledComponents("v")),
    _grads(_op_num),
    _factor(getParam<Real>("grains_per_side") * getParam<Real>("op_range"))
{
  // make sure user input is valid
  if (MooseUtils::absoluteFuzzyEqual(_factor, 0.0))
    mooseError("Neither grains_per_side nor op_range may be zero.");

  // Loop over variables (ops)
  for (auto op_index = decltype(_op_num)(0); op_index < _op_num; ++op_index)
    _grads[op_index] = &coupledGradient("v", op_index);
}

Real
GrainBoundaryArea::computeQpIntegral()
{
  Real grad_sum = 0.0;
  for (auto grad : _grads)
    grad_sum += (*grad)[_qp].norm();
  return grad_sum;
}

Real
GrainBoundaryArea::getValue()
{
  return ElementIntegralPostprocessor::getValue() / _factor;
}
