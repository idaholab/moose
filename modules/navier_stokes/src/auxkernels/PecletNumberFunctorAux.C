//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PecletNumberFunctorAux.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PecletNumberFunctorAux);

InputParameters
PecletNumberFunctorAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes the Peclet number: u*L/alpha.");
  params.addRequiredParam<MooseFunctorName>(NS::speed, "The fluid speed");
  params.addRequiredParam<MooseFunctorName>(
      NS::thermal_diffusivity,
      "The fluid thermal diffusivity, or if using this object for evaluating mass transfer, the "
      "mass diffusivity. Regardless of which, this should have units of length^2/time");
  return params;
}

PecletNumberFunctorAux::PecletNumberFunctorAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _speed(getFunctor<ADReal>(NS::speed)),
    _alpha(getFunctor<ADReal>(NS::thermal_diffusivity)),
    _use_qp_arg(dynamic_cast<MooseVariableFE<Real> *>(&_var))
{
  if (!_use_qp_arg && !dynamic_cast<MooseVariableFV<Real> *>(&_var))
    paramError(
        "variable",
        "The variable must be a non-vector, non-array finite-volume/finite-element variable.");

  if (isNodal())
    mooseError("This AuxKernel only supports Elemental fields");
}

Real
PecletNumberFunctorAux::computeValue()
{
  using MetaPhysicL::raw_value;
  const auto state = determineState();

  if (_use_qp_arg)
  {
    const auto qp_arg = std::make_tuple(_current_elem, _qp, _qrule);
    return _current_elem->hmax() * raw_value(_speed(qp_arg, state)) /
           raw_value(_alpha(qp_arg, state));
  }
  else
  {
    const auto elem_arg = makeElemArg(_current_elem);
    return _current_elem->hmax() * raw_value(_speed(elem_arg, state)) /
           raw_value(_alpha(elem_arg, state));
  }
}
