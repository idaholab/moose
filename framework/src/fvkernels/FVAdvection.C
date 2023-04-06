//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVAdvection.h"

registerADMooseObject("MooseApp", FVAdvection);

InputParameters
FVAdvection::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription(
      "Residual contribution from advection operator for finite volume method.");
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  MooseEnum advected_interp_method("average upwind skewness-corrected", "upwind");

  params.addParam<MooseEnum>(
      "advected_interp_method",
      advected_interp_method,
      "The interpolation to use for the advected quantity. Options are "
      "'upwind', 'average' and 'skewness-corrected', with the default being 'upwind'.");
  return params;
}

FVAdvection::FVAdvection(const InputParameters & params)
  : FVFluxKernel(params), _velocity(getParam<RealVectorValue>("velocity"))
{
  using namespace Moose::FV;

  const auto & advected_interp_method = getParam<MooseEnum>("advected_interp_method");
  if (advected_interp_method == "average")
    _advected_interp_method = InterpMethod::Average;
  else if (advected_interp_method == "upwind")
    _advected_interp_method = InterpMethod::Upwind;
  else if (advected_interp_method == "skewness-corrected")
    _advected_interp_method = InterpMethod::SkewCorrectedAverage;
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(advected_interp_method));
}

ADReal
FVAdvection::computeQpResidual()
{
  const bool elem_is_upwind = _velocity * _normal >= 0;
  const auto face =
      makeFace(*_face_info, Moose::FV::limiterType(_advected_interp_method), elem_is_upwind);
  ADReal u_interface = _var(face, determineState());

  return _normal * _velocity * u_interface;
}
