//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVNormalVelocityFunctorDirichletBC.h"

registerMooseObject("NavierStokesApp", LinearFVNormalVelocityFunctorDirichletBC);

InputParameters
LinearFVNormalVelocityFunctorDirichletBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionFunctorDirichletBC::validParams();
  params.addClassDescription(
      "Adds a dirichlet BC for a velocity parallel to the normal direction. A positive dirichlet "
      "value would denote outflow, while negative denotes inflow.");
  params.renameParam("functor", "normal_velocity", "The velocity in the normal direction");
  MooseEnum component("x y z");
  params.addRequiredParam<MooseEnum>(
      "component",
      component,
      "The velocity component this object is acting on. We will multiply the prescribed normal "
      "velocity by the corresponding face normal component");
  return params;
}

LinearFVNormalVelocityFunctorDirichletBC::LinearFVNormalVelocityFunctorDirichletBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionFunctorDirichletBC(parameters),
    _component((getParam<MooseEnum>("component").getEnum<Component>()))
{
}

Real
LinearFVNormalVelocityFunctorDirichletBC::computeBoundaryValue() const
{
  return LinearFVAdvectionDiffusionFunctorDirichletBC::computeBoundaryValue() *
         _current_face_info->normal()(_component);
}
