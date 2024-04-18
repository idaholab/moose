//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorCoordinatesFunctionAux.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctorCoordinatesFunctionAux);

InputParameters
FunctorCoordinatesFunctionAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Auxiliary Kernel that creates and updates a field variable by "
      "sampling a function with functors (variables, functions, others) as the coordinates.");
  params.addRequiredParam<FunctionName>("function", "The function to use as the value");
  params.addRequiredParam<MooseFunctorName>(
      "x_functor", "The functor to use for the X coordinate function argument");
  params.addRequiredParam<MooseFunctorName>(
      "y_functor", "The functor to use for the Y coordinate function argument");
  params.addRequiredParam<MooseFunctorName>(
      "z_functor", "The functor to use for the Z coordinate function argument");
  params.addRequiredParam<MooseFunctorName>("t_functor",
                                            "The functor to use for the time function argument");
  params.addParam<MooseFunctorName>("factor", 1, "A factor to apply on the functor");

  return params;
}

FunctorCoordinatesFunctionAux::FunctorCoordinatesFunctionAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _func(getFunction("function")),
    _x_functor(getFunctor<Real>("x_functor")),
    _y_functor(getFunctor<Real>("y_functor")),
    _z_functor(getFunctor<Real>("z_functor")),
    _t_functor(getFunctor<Real>("t_functor")),
    _factor(getFunctor<Real>("factor")),
    _is_fe(dynamic_cast<MooseVariableFE<Real> *>(&_var))
{
  if (!_is_fe && !dynamic_cast<MooseVariableFV<Real> *>(&_var) &&
      !dynamic_cast<MooseLinearVariableFV<Real> *>(&_var))
    paramError(
        "variable",
        "The variable must be a non-vector, non-array finite-volume/finite-element variable.");
}

Real
FunctorCoordinatesFunctionAux::computeValue()
{
  const auto state = determineState();
  if (isNodal())
  {
    const Moose::NodeArg node_arg = {_current_node, Moose::INVALID_BLOCK_ID};
    return _factor(node_arg, state) * _func.value(_t_functor(node_arg, state),
                                                  Point(_x_functor(node_arg, state),
                                                        _y_functor(node_arg, state),
                                                        _z_functor(node_arg, state)));
  }
  else if (_is_fe)
  {
    const Moose::ElemQpArg qp_arg = {_current_elem, _qp, _qrule, _q_point[_qp]};
    return _factor(qp_arg, state) * _func.value(_t_functor(qp_arg, state),
                                                Point(_x_functor(qp_arg, state),
                                                      _y_functor(qp_arg, state),
                                                      _z_functor(qp_arg, state)));
  }
  else
  {
    const auto elem_arg = makeElemArg(_current_elem);
    return _factor(elem_arg, state) * _func.value(_t_functor(elem_arg, state),
                                                  Point(_x_functor(elem_arg, state),
                                                        _y_functor(elem_arg, state),
                                                        _z_functor(elem_arg, state)));
  }
}
