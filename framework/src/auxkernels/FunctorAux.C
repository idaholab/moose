//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorAux.h"

registerMooseObject("MooseApp", FunctorAux);
registerMooseObjectRenamed("MooseApp", FunctorElementalAux, "10/14/2024 00:00", FunctorAux);
registerMooseObjectRenamed("MooseApp", ADFunctorElementalAux, "10/14/2024 00:00", FunctorAux);

InputParameters
FunctorAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Evaluates a functor (variable, function or functor material property) on the current "
      "element, quadrature point, or node.");
  params.addRequiredParam<MooseFunctorName>("functor", "The functor to evaluate");
  params.addParam<MooseFunctorName>("factor", 1, "A factor to apply on the functor");
  return params;
}

FunctorAux::FunctorAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _functor(getFunctor<Real>("functor")),
    _factor(getFunctor<Real>("factor")),
    _is_standard_fe(dynamic_cast<MooseVariableFE<Real> *>(&_var)),
    _is_standard_fv(dynamic_cast<MooseVariableFV<Real> *>(&_var) ||
                    dynamic_cast<MooseLinearVariableFV<Real> *>(&_var))
{
  if (!_is_standard_fe && !_is_standard_fv)
    paramError(
        "variable",
        "The variable must be a non-vector, non-array finite-volume/finite-element variable.");
}

Real
FunctorAux::computeValue()
{
  const auto state = determineState();
  if (isNodal())
  {
    const Moose::NodeArg node_arg = {_current_node, Moose::INVALID_BLOCK_ID};
    return _factor(node_arg, state) * _functor(node_arg, state);
  }
  else if (_is_standard_fe)
  {
    const Moose::ElemQpArg qp_arg = {_current_elem, _qp, _qrule, _q_point[_qp]};
    return _factor(qp_arg, state) * _functor(qp_arg, state);
  }
  else
  {
    const auto elem_arg = makeElemArg(_current_elem);
    return _factor(elem_arg, state) * _functor(elem_arg, state);
  }
}
