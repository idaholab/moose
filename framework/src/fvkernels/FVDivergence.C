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
  params.addClassDescription(
      "Computes the residual coming from the divergence of a vector field "
      "that can be represented as a functor. Furthermore, we assume that this vector field does "
      "not depend on the solution, therefore the derivatives are not propagated.");
  params.addRequiredParam<MooseFunctorName>(
      "vector_field", "The name of the vector field whose divergence is added to the residual.");
  return params;
}

FVDivergence::FVDivergence(const InputParameters & params)
  : FVFluxKernel(params), _vector_field(getFunctor<ADRealVectorValue>("vector_field"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError(
      "FVDivergence is not supported by local AD indexing. In order to use this object, please run "
      "the configure script in the root MOOSE directory with the configure option "
      "'--with-ad-indexing-type=global'. Note that global indexing is now the default "
      "configuration for AD indexing type.");
#endif
}

ADReal
FVDivergence::computeQpResidual()
{
  using namespace Moose::FV;

  ADRealVectorValue face_value;

  // If we are on internal faces, we interpolate the vector field as usual
  if (_var.isInternalFace(*_face_info))
    interpolate(Moose::FV::InterpMethod::SkewCorrectedAverage,
                face_value,
                _vector_field(elemFromFace()),
                _vector_field(neighborFromFace()),
                *_face_info,
                true);
  // Else we just use the boundary values
  else
  {
    const auto face = singleSidedFaceArg();
    face_value = _vector_field(face);
  }

  return face_value * _face_info->normal();
}
