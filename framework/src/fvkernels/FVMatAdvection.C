//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMatAdvection.h"

registerADMooseObject("MooseApp", FVMatAdvection);

InputParameters
FVMatAdvection::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription("Computes the residual of advective term using finite volume method.");
  params.addRequiredParam<MooseFunctorName>("vel", "advection velocity");
  params.addParam<MooseFunctorName>(
      "advected_quantity",
      "An optional parameter for specifying an advected quantity from a material property. If this "
      "is not specified, then the advected quantity will simply be the variable that this object "
      "is acting on");

  MooseEnum advected_interp_method("average upwind skewness-corrected", "upwind");
  params.addParam<MooseEnum>(
      "advected_interp_method",
      advected_interp_method,
      "The interpolation to use for the advected quantity. Options are "
      "'upwind', 'average', and 'skewness-corrected' with the default being 'upwind'.");
  return params;
}

FVMatAdvection::FVMatAdvection(const InputParameters & params)
  : FVFluxKernel(params),
    _vel(getFunctor<ADRealVectorValue>("vel")),
    _adv_quant(getFunctor<ADReal>(isParamValid("advected_quantity") ? "advected_quantity"
                                                                    : variable().name()))
{
  using namespace Moose::FV;

  const auto & advected_interp_method = getParam<MooseEnum>("advected_interp_method");
  if (advected_interp_method == "average")
    _advected_interp_method = InterpMethod::Average;
  else if (advected_interp_method == "skewness-corrected")
    _advected_interp_method = Moose::FV::InterpMethod::SkewCorrectedAverage;
  else if (advected_interp_method == "upwind")
    _advected_interp_method = InterpMethod::Upwind;
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(advected_interp_method));
}

ADReal
FVMatAdvection::computeQpResidual()
{
  using namespace Moose::FV;

  const auto v = _vel(makeFace(*_face_info,
                               LimiterType::CentralDifference,
                               true,
                               _advected_interp_method == InterpMethod::SkewCorrectedAverage),
                      determineState());
  const auto adv_quant_interface =
      _adv_quant(makeFace(*_face_info,
                          limiterType(_advected_interp_method),
                          MetaPhysicL::raw_value(v) * _normal > 0,
                          _advected_interp_method == InterpMethod::SkewCorrectedAverage),
                 determineState());

  return _normal * v * adv_quant_interface;
  ;
}
