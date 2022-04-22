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

  params.addCoupledVar("v", "Variable to take the time derivative of");
  params.addParam<MooseFunctorName>("functor", "Functor to take the time derivative of");
  params.addParam<MooseFunctorName>("factor", 1, "Factor to multiply the time derivative by");

  return params;
}

template <bool is_ad>
TimeDerivativeAuxTempl<is_ad>::TimeDerivativeAuxTempl(const InputParameters & parameters)
  : AuxKernel(parameters),
    _v_dot(nullptr),
    _v_var(isParamValid("v") ? getVarHelper<MooseVariableField<Real>>("v", 0) : nullptr),
    _functor(isParamValid("functor") ? &getFunctor<GenericReal<is_ad>>("functor") : nullptr),
    _factor(getFunctor<GenericReal<is_ad>>("factor")),
    _use_qp_arg(dynamic_cast<MooseVariableFE<Real> *>(&_var))
{
  // Handle missing functor/coupledDot implementations to try to error as rarely as possible
  if (isParamValid("v"))
  {
    if (dynamic_cast<const MooseVariableFE<Real> *>(_v_var))
    {
      if (!_use_qp_arg)
        mooseWarning("'v' argument is a finite element variable but 'variable' is not.");
      _v_dot = &coupledDot("v");
    }
    else
    {
      if (_use_qp_arg)
        mooseWarning("'variable' argument is a finite element variable but 'v' is not.");
      _functor = &getFunctor<GenericReal<is_ad>>("v");
    }
  }

  if ((!_functor && !_v_dot) || (_functor && _v_dot))
    mooseError("A variable or a functor parameter should be provided to the "
               "TimeDerivativeAuxTempl<is_ad>");

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

    if (_v_dot)
      return raw_value(_factor(qp_arg)) * (*_v_dot)[_qp];
    else
      return raw_value(_factor(qp_arg)) * raw_value((*_functor).dot(qp_arg));
  }
  else
  {
    const auto elem_arg = makeElemArg(_current_elem);

    if (_v_dot)
      return raw_value(_factor(elem_arg)) * (*_v_dot)[_qp];
    else
      return raw_value(_factor(elem_arg)) * raw_value((*_functor).dot(elem_arg));
  }
}
