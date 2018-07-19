/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NodalGradientAux.h"

registerMooseObject("MooseApp", NodalGradientAux);

template <>
InputParameters
validParams<NodalGradientAux>()
{
  InputParameters params = validParams<NodalPatchRecovery>();
  params.addClassDescription(
      "using nodal patch recovery to construct a smooth nodal field of gradient");
  params.addRequiredCoupledVar("coupled_var", "variable to calculate gradient from");
  params.addRequiredParam<unsigned>("component", "component of the gradient");
  return params;
}

NodalGradientAux::NodalGradientAux(const InputParameters & parameters)
  : NodalPatchRecovery(parameters),
    _component(getParam<unsigned>("component")),
    _grad(coupledGradient("coupled_var"))
{
}

Real
NodalGradientAux::computeValue()
{
  return _grad[_qp](_component);
}
