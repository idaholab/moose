//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeDerivativeAux.h"

registerMooseObject("MooseApp", TimeDerivativeAux);
registerMooseObject("MooseApp", ADTimeDerivativeAux);

template <bool is_ad>
InputParameters
TimeDerivativeAuxTempl<is_ad>::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Returns the time derivative of the specified variable/functor as an auxiliary variable.");

  params.addParam<MooseFunctorName>("functor", "Functor to take the time derivative of");
  params.addParam<MooseFunctorName>("factor", 1, "Factor to multiply the time derivative by");

  return params;
}

template <bool is_ad>
TimeDerivativeAuxTempl<is_ad>::TimeDerivativeAuxTempl(const InputParameters & parameters)
  : AuxKernel(parameters),
    _functor(getFunctor<GenericReal<is_ad>>("functor")),
    _factor(getFunctor<GenericReal<is_ad>>("factor")),
    _use_qp_arg(dynamic_cast<MooseVariableFE<Real> *>(&_var))
{
  const auto functor_name = getParam<MooseFunctorName>("functor");
  if (_subproblem.hasVariable(functor_name))
  {
    const auto * functor_var = &_subproblem.getVariable(_tid, functor_name);
    if (dynamic_cast<const MooseVariableFE<Real> *>(&_var) &&
        !dynamic_cast<const MooseVariableFE<Real> *>(functor_var))
      mooseWarning("'variable' argument is a finite element variable but 'functor' is not.");
    if (!dynamic_cast<const MooseVariableFE<Real> *>(&_var) &&
        dynamic_cast<const MooseVariableFE<Real> *>(functor_var))
      mooseWarning("'functor' argument is a finite element variable but 'variable' is not.");
  }
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
}

template <bool is_ad>
Real
TimeDerivativeAuxTempl<is_ad>::computeValue()
{
  using MetaPhysicL::raw_value;

  if (_use_qp_arg)
  {
    const auto qp_arg = std::make_tuple(_current_elem, _qp, _qrule);
    return raw_value(_factor(qp_arg)) * raw_value(_functor.dot(qp_arg));
  }
  else
  {
    const auto elem_arg = makeElemArg(_current_elem);
    return raw_value(_factor(elem_arg)) * raw_value(_functor.dot(elem_arg));
  }
}
