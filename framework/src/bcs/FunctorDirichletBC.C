//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorDirichletBC.h"

registerMooseObject("MooseApp", FunctorDirichletBC);

InputParameters
FunctorDirichletBC::validParams()
{
  InputParameters params = ADDirichletBCBase::validParams();

  params.addClassDescription("Imposes the Dirichlet boundary condition "
                             "$u(t,\\vec{x})=h(t,\\vec{x})$, "
                             "where $h$ is a functor and can have complex dependencies.");

  params.addRequiredParam<MooseFunctorName>("functor", "The functor to impose");
  params.addParam<MooseFunctorName>(
      "coefficient", 1.0, "An optional functor coefficient to multiply the imposed functor");

  return params;
}

FunctorDirichletBC::FunctorDirichletBC(const InputParameters & parameters)
  : ADDirichletBCBaseTempl<Real>(parameters),
    _functor(getFunctor<ADReal>("functor")),
    _coef(getFunctor<ADReal>("coefficient"))
{
}

ADReal
FunctorDirichletBC::computeQpValue()
{
  const Moose::NodeArg space_arg = {_current_node, Moose::INVALID_BLOCK_ID};
  const Moose::StateArg time_arg = Moose::currentState();
  return _coef(space_arg, time_arg) * _functor(space_arg, time_arg);
}
