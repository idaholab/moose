//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorNeumannBC.h"

registerMooseObject("MooseApp", FunctorNeumannBC);

InputParameters
FunctorNeumannBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();

  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=h(t,\\vec{x})$, "
                             "where $h$ is a functor.");

  params.addRequiredParam<MooseFunctorName>("functor", "The functor to impose");
  params.addParam<MooseFunctorName>(
      "coefficient", 1.0, "An optional functor coefficient to multiply the imposed functor");
  params.addParam<bool>("flux_is_inward",
                        true,
                        "Set to true if a positive evaluation of the provided functor corresponds "
                        "to a flux in the inward direction; else the outward direction");

  return params;
}

FunctorNeumannBC::FunctorNeumannBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _functor(getFunctor<ADReal>("functor")),
    _coef(getFunctor<ADReal>("coefficient")),
    _sign(getParam<bool>("flux_is_inward") ? -1.0 : 1.0)
{
}

ADReal
FunctorNeumannBC::computeQpResidual()
{
  const Moose::ElemSideQpArg space_arg = {_current_elem, _current_side, _qp, _qrule, _q_point[_qp]};
  return _sign * _coef(space_arg, Moose::currentState()) *
         _functor(space_arg, Moose::currentState()) * _test[_i][_qp];
}
