//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVAdvectionFunctionBC.h"
#include "Function.h"

registerMooseObject("MooseApp", FVAdvectionFunctionBC);

InputParameters
FVAdvectionFunctionBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addRequiredParam<FunctionName>("exact_solution", "The exact solution.");
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "arising from integration by parts of an advection operator, and "
                             "where the exact solution can be specified.");
  MooseEnum advected_interp_method("average upwind", "upwind");

  params.addParam<MooseEnum>("advected_interp_method",
                             advected_interp_method,
                             "The interpolation to use for the advected quantity. Options are "
                             "'upwind' and 'average', with the default being 'upwind'.");
  return params;
}

FVAdvectionFunctionBC::FVAdvectionFunctionBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _exact_solution(getFunction("exact_solution")),
    _velocity(getParam<RealVectorValue>("velocity"))
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
FVAdvectionFunctionBC::computeQpResidual()
{
  ADReal u_face;
  interpolate(
      _advected_interp_method,
      u_face,
      _u[_qp],
      _exact_solution.value(_t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid()),
      _velocity,
      *_face_info,
      true);
  return _normal * _velocity * u_face;
}
