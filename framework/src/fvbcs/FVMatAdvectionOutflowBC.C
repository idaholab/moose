//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMatAdvectionOutflowBC.h"

registerADMooseObject("MooseApp", FVMatAdvectionOutflowBC);

InputParameters
FVMatAdvectionOutflowBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addRequiredParam<MaterialPropertyName>("vel", "advection velocity");
  params.addParam<MaterialPropertyName>(
      "advected_quantity",
      "An optional parameter for specifying an advected quantity from a material property. If this "
      "is not specified, then the advected quantity will simply be the variable that this object "
      "is acting on");
  params.addClassDescription(
      "Outflow boundary condition taking the advected quantity from a material property");
  MooseEnum advected_interp_method("average upwind", "upwind");

  params.addDeprecatedParam<MooseEnum>(
      "advected_interp_method",
      advected_interp_method,
      "The interpolation to use for the advected quantity. Options are "
      "'upwind' and 'average', with the default being 'upwind'.",
      "There's no such thing as interpolation on a boundary");
  return params;
}

FVMatAdvectionOutflowBC::FVMatAdvectionOutflowBC(const InputParameters & params)
  : FVFluxBC(params),
    _vel(getFunctorMaterialProperty<ADRealVectorValue>("vel")),
    _adv_quant(isParamValid("advected_quantity")
                   ? static_cast<const FunctorInterface<ADReal> &>(
                         getFunctorMaterialProperty<ADReal>("advected_quantity"))
                   : static_cast<const FunctorInterface<ADReal> &>(variable()))
{
}

ADReal
FVMatAdvectionOutflowBC::computeQpResidual()
{
  _v = _vel(std::make_tuple(_face_info, nullptr, true));
  const auto adv_quant_boundary =
      _adv_quant(std::make_tuple(_face_info, nullptr, _v * _face_info->normal() > 0));
  return _normal * _v * adv_quant_boundary;
}
