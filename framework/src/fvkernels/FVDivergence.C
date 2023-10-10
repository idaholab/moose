//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVDivergence.h"

registerMooseObject("MooseApp", FVDivergence);

InputParameters
FVDivergence::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription("Computes the residual coming from the divergence of a vector field"
                             "that can be represented as a functor.");
  params.addRequiredParam<MooseFunctorName>(
      "vector_field", "The name of the vector field whose divergence is added to the residual.");
  return params;
}

FVDivergence::FVDivergence(const InputParameters & params)
  : FVFluxKernel(params), _vector_field(getFunctor<ADRealVectorValue>("vector_field"))
{
}

ADReal
FVDivergence::computeQpResidual()
{
  const auto face =
      makeFace(*_face_info, Moose::FV::limiterType(Moose::FV::InterpMethod::Average), true);
  const auto vector = _vector_field(face, determineState());
  return -1.0 * (vector * _normal);
}
