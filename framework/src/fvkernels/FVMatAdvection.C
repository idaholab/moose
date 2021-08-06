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

  params.addParam<MooseEnum>("advected_interp_method",
                             advected_interp_method,
                             "The interpolation to use for the advected quantity. Options are "
                             "'upwind' and 'average', with the default being 'upwind'.");
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
  ADReal adv_quant_interface;
  ADRealVectorValue v;

  using namespace Moose::FV;

  const auto elem_face = elemFromFace();
  const auto neighbor_face = neighborFromFace();

  // Currently only Average is supported for the velocity
  interpolate(InterpMethod::Average, v, _vel(elem_face), _vel(neighbor_face), *_face_info, true);

  interpolate(_advected_interp_method,
              adv_quant_interface,
              _adv_quant(elem_face),
              _adv_quant(neighbor_face),
              v,
              *_face_info,
              true);
  return _normal * v * adv_quant_interface;
}
