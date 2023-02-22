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
  const auto face = makeFace(*_face_info, limiterType(Moose::FV::InterpMethod::Average), false);
  // We use this on the right hand side as a source so in the residual this will be a sink (ffactor
  // of -1)
  return -1.0 * (_vector_field(face) * _normal);
}
