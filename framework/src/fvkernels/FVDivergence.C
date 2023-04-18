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
      "Computes the residual coming from the divergence of a vector field"
      "that can be represented as a functor. Furthermore, we assume that this vector field does "
      "not depend on the solution, therefore the derivatives are not propagated and this term can "
      "only be used as a source term on a right hand side.");
  params.addRequiredParam<MooseFunctorName>(
      "vector_field", "The name of the vector field whose divergence is added to the residual.");
  return params;
}

FVDivergence::FVDivergence(const InputParameters & params)
  : FVFluxKernel(params), _vector_field(getFunctor<RealVectorValue>("vector_field"))
{
}

ADReal
FVDivergence::computeQpResidual()
{
  using namespace Moose::FV;

  // const auto face = makeFace(*_face_info, limiterType(Moose::FV::InterpMethod::Average), false);
  const auto face =
      makeFace(*_face_info, Moose::FV::limiterType(Moose::FV::InterpMethod::Average), true);
  RealVectorValue vector = _vector_field(face);

  // // If we are on internal faces, we interpolate the diffusivity as usual
  // if (_var.isInternalFace(*_face_info))
  //   interpolate(Moose::FV::InterpMethod::Average,
  //               vector,
  //               _vector_field(elemArg()),
  //               _vector_field(neighborArg()),
  //               *_face_info,
  //               true);
  // // Else we just use the boundary values (which depend on how the diffusion
  // // coefficient is constructed)
  // else
  // {
  //   const auto face = singleSidedFaceArg();
  //   vector = _vector_field(face);
  // }

  // // We use this on the right hand side as a source so in the residual this will be a sink (factor
  // // of -1)
  // std::cout << _face_info->faceCentroid() << " " <<  (vector * _normal)*_face_info->faceArea() << std::endl;
  return -1.0 * (vector * _normal);
}
