//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVLimitedVectorAdvection.h"
#include "Limiter.h"

registerMooseObject("MooseTestApp", FVLimitedVectorAdvection);

using namespace Moose::FV;

InputParameters
FVLimitedVectorAdvection::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  params.addParam<MooseEnum>(
      "limiter", moose_limiter_type, "The limiter to use for the advected quantity.");
  params.addRequiredParam<MooseFunctorName>("x_functor", "The x-component functor.");
  params.addRequiredParam<MooseFunctorName>("y_functor", "The y-component functor.");
  params.addRequiredParam<MooseFunctorName>("z_functor", "The z-component functor.");
  params.addRequiredParam<unsigned int>(
      "component",
      "The component at which we will index the evaluated vector functor to populate our residual");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

FVLimitedVectorAdvection::FVLimitedVectorAdvection(const InputParameters & params)
  : FVFluxKernel(params),
    _velocity(getParam<RealVectorValue>("velocity")),
    _limiter_type(LimiterType(int(getParam<MooseEnum>("limiter")))),
    _vector(name() + "_vector_composite",
            getFunctor<ADReal>("x_functor"),
            getFunctor<ADReal>("y_functor"),
            getFunctor<ADReal>("z_functor")),
    _index(getParam<unsigned int>("component"))
{
}

ADReal
FVLimitedVectorAdvection::computeQpResidual()
{
  const bool elem_is_upwind = _velocity * _normal >= 0;
  const auto face = makeFace(*_face_info, _limiter_type, elem_is_upwind);

  ADReal phi_f;

  if (_var.isInternalFace(*_face_info))
    phi_f = interpolate(_vector, face, Moose::currentState())(_index);
  else
    phi_f = _var(face, Moose::currentState());

  return _normal * _velocity * phi_f;
}
