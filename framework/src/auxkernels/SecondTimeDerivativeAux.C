//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SecondTimeDerivativeAux.h"

registerMooseObject("MooseApp", SecondTimeDerivativeAux);

InputParameters
SecondTimeDerivativeAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Returns the second order time derivative of the specified variable "
                             "as an auxiliary variable.");

  params.addRequiredCoupledVar("v", "Variable to take the second time derivative of");
  params.addParam<MooseFunctorName>(
      "factor", 1, "Factor to multiply the second time derivative by");

  return params;
}

SecondTimeDerivativeAux::SecondTimeDerivativeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _v(coupledDotDot("v")),
    _factor(getFunctor<Real>("factor")),
    _use_qp_arg(dynamic_cast<MooseVariableFE<Real> *>(&_var))
{
  const auto v_name = coupledName("v");
  if (_subproblem.hasVariable(v_name))
  {
    const auto * functor_var = &_subproblem.getVariable(_tid, v_name);
    if (dynamic_cast<const MooseVariableFE<Real> *>(&_var) &&
        !dynamic_cast<const MooseVariableFE<Real> *>(functor_var))
      mooseWarning("'variable' argument is a finite element variable but 'v' is not.");
    if (!dynamic_cast<const MooseVariableFE<Real> *>(&_var) &&
        dynamic_cast<const MooseVariableFE<Real> *>(functor_var))
      mooseWarning("'v' argument is a finite element variable but 'variable' is not.");
  }
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
}

Real
SecondTimeDerivativeAux::computeValue()
{
  if (_use_qp_arg)
  {
    const auto qp_arg = std::make_tuple(_current_elem, _qp, _qrule);
    return _factor(qp_arg, determineState()) * _v[_qp];
  }
  else
  {
    const auto elem_arg = makeElemArg(_current_elem);
    return _factor(elem_arg, determineState()) * _v[_qp];
  }
}
