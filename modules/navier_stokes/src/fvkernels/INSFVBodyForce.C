//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVBodyForce.h"

registerMooseObject("NavierStokesApp", INSFVBodyForce);

InputParameters
INSFVBodyForce::validParams()
{
  auto params = FVElementalKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  params.addClassDescription("Body force that contributes to the Rhie-Chow interpolation");
  params.addParam<Real>("scaling_factor", 1.0, "Coefficient to multiply by the body force term");
  params.addParam<MooseFunctorName>("functor", "1", "A functor that describes the body force");
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
  params.declareControllable("scaling_factor");
  return params;
}

INSFVBodyForce::INSFVBodyForce(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    INSFVMomentumResidualObject(*this),
    _scale(getParam<Real>("scaling_factor")),
    _functor(getFunctor<ADReal>("functor")),
    _postprocessor(getPostprocessorValue("postprocessor"))
{
}

ADReal
INSFVBodyForce::computeQpResidual()
{
  return -_scale * _postprocessor * _functor(makeElemArg(_current_elem), determineState());
}
