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
  params.addRequiredParam<MaterialPropertyName>("vel", "advection velocity");
  params.addParam<MaterialPropertyName>(
      "advected_quantity",
      "An optional parameter for specifying an advected quantity from a material property. If this "
      "is not specified, then the advected quantity will simply be the variable that this object "
      "is acting on");

  MooseEnum advected_interp_method("average upwind", "upwind");

  params.addParam<MooseEnum>("advected_interp_method",
                             advected_interp_method,
                             "The interpolation to use for the advected quantity. Options are "
                             "'upwind' and 'average', with the default being 'upwind'.");
  return params;
}

FVMatAdvection::FVMatAdvection(const InputParameters & params)
  : FVFluxKernel(params),
    _vel(getFunctorMaterialProperty<ADRealVectorValue>("vel")),
    _adv_quant(isParamValid("advected_quantity")
                   ? static_cast<const FunctorInterface<ADReal> &>(
                         getFunctorMaterialProperty<ADReal>("advected_quantity"))
                   : static_cast<const FunctorInterface<ADReal> &>(variable()))
{
  using namespace Moose::FV;

  _cd_limiter = Limiter::build(LimiterType::CentralDifference);
  const auto & advected_interp_method = getParam<MooseEnum>("advected_interp_method");
  if (advected_interp_method == "average")
    _limiter = Limiter::build(LimiterType::CentralDifference);
  else if (advected_interp_method == "upwind")
    _limiter = Limiter::build(LimiterType::Upwind);
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(advected_interp_method));
}

ADReal
FVMatAdvection::computeQpResidual()
{
  using namespace Moose::FV;

  _v = _vel(std::make_tuple(_face_info, _cd_limiter.get(), /*this doesn't matter for cd*/ true));
  const auto adv_quant_interface =
      _adv_quant(std::make_tuple(_face_info, _limiter.get(), _v * _face_info->normal() > 0));

  return _normal * _v * adv_quant_interface;
}
