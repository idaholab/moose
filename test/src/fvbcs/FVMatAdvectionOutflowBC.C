//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMatAdvectionOutflowBC.h"

registerMooseObject("MooseTestApp", FVMatAdvectionOutflowBC);

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

  params.addParam<MooseEnum>("advected_interp_method",
                             advected_interp_method,
                             "The interpolation to use for the advected quantity. Options are "
                             "'upwind' and 'average', with the default being 'upwind'.");
  return params;
}

FVMatAdvectionOutflowBC::FVMatAdvectionOutflowBC(const InputParameters & params)
  : FVFluxBC(params),
    _vel(getFunctor<ADRealVectorValue>("vel")),
    _adv_quant(getFunctor<ADReal>(isParamValid("advected_quantity") ? "advected_quantity"
                                                                    : variable().name()))
{
  using namespace Moose::FV;

  const auto & advected_interp_method = getParam<MooseEnum>("advected_interp_method");
  if (advected_interp_method == "average")
    _advected_interp_method = InterpMethod::Average;
  else if (advected_interp_method == "upwind")
    _advected_interp_method = InterpMethod::Upwind;
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(advected_interp_method));
}

ADReal
FVMatAdvectionOutflowBC::computeQpResidual()
{
  const auto ssf = singleSidedFaceArg();
  ADRealVectorValue v = _vel(ssf, Moose::currentState());
  ADReal adv_quant_boundary = _adv_quant(ssf, Moose::currentState());

  return _normal * v * adv_quant_boundary;
}
