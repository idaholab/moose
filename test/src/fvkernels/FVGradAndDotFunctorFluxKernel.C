//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVGradAndDotFunctorFluxKernel.h"

registerMooseObject("MooseTestApp", FVGradAndDotFunctorFluxKernel);

InputParameters
FVGradAndDotFunctorFluxKernel::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addRequiredParam<MooseFunctorName>("grad_functor", "A functor representing a gradient.");
  params.addRequiredParam<MooseFunctorName>("dot_functor",
                                            "A functor representing a time derivative.");
  params.addRequiredParam<MooseFunctorName>("value_functor", "A functor representing a value.");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

FVGradAndDotFunctorFluxKernel::FVGradAndDotFunctorFluxKernel(const InputParameters & params)
  : FVFluxKernel(params),
    _value(getFunctor<ADReal>("value_functor")),
    _gradient(getFunctor<ADRealVectorValue>("grad_functor")),
    _dot(getFunctor<ADReal>("dot_functor")),
    _velocity(1, 0, 0)
{
}

ADReal
FVGradAndDotFunctorFluxKernel::computeQpResidual()
{
  const Real sign = _velocity * _normal > 0 ? 1 : -1;
  // single-sided face
  Moose::SingleSidedFaceArg ssf{_face_info,
                                Moose::FV::LimiterType::CentralDifference,
                                true,
                                false,
                                false,
                                _face_info->elem().subdomain_id()};
  return sign * (_value(ssf) + _gradient(ssf)(0) + _dot(ssf));
}
