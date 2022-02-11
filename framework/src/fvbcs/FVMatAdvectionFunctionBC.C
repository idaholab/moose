//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMatAdvectionFunctionBC.h"
#include "Function.h"

registerADMooseObject("MooseApp", FVMatAdvectionFunctionBC);

InputParameters
FVMatAdvectionFunctionBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Imposes the integrated boundary condition "
                             "arising from integration by parts of an advection operator, where "
                             "the advected quantity is computed in a material, and "
                             "where the exact solution can be specified.");
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

  params.addRequiredParam<FunctionName>(
      "flux_variable_exact_solution",
      "The exact solution for the *flux* variable (may not necessarily be the *variable*, e.g. the "
      "flux variable could be rhou, while the variable is u).");
  params.addRequiredParam<FunctionName>("vel_x_exact_solution",
                                        "The function describing the x velocity.");
  params.addParam<FunctionName>("vel_y_exact_solution", "The function describing the y velocity.");
  params.addParam<FunctionName>("vel_z_exact_solution", "The function describing the z velocity.");

  return params;
}

FVMatAdvectionFunctionBC::FVMatAdvectionFunctionBC(const InputParameters & params)
  : FVFluxBC(params),
    _vel(getFunctor<ADRealVectorValue>("vel")),
    _adv_quant(getFunctor<ADReal>(isParamValid("advected_quantity") ? "advected_quantity"
                                                                    : variable().name())),
    _flux_variable_exact_solution(isParamValid("flux_variable_exact_solution")
                                      ? &getFunction("flux_variable_exact_solution")
                                      : nullptr),
    _vel_x_exact_solution(getFunction("vel_x_exact_solution")),
    _vel_y_exact_solution(
        isParamValid("vel_y_exact_solution") ? &getFunction("vel_y_exact_solution") : nullptr),
    _vel_z_exact_solution(
        isParamValid("vel_z_exact_solution") ? &getFunction("vel_z_exact_solution") : nullptr)

{
  const auto & advected_interp_method = getParam<MooseEnum>("advected_interp_method");
  if (advected_interp_method == "average")
    _advected_interp_method = Moose::FV::InterpMethod::Average;
  else if (advected_interp_method == "upwind")
    _advected_interp_method = Moose::FV::InterpMethod::Upwind;
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(advected_interp_method));

  if (_mesh.dimension() >= 2 && !_vel_y_exact_solution)
    mooseError("If the mesh dimension is at least 2, then the 'vel_y_exact_solution' parameter "
               "must be provided");

  if (_mesh.dimension() >= 3 && !_vel_z_exact_solution)
    mooseError(
        "If the mesh dimension is 3, then the 'vel_z_exact_solution' parameter must be provided");
}

ADReal
FVMatAdvectionFunctionBC::computeQpResidual()
{
  ADReal flux_var_face;
  ADRealVectorValue v_face;

  mooseAssert(
      _flux_variable_exact_solution,
      "_flux_variable_exact_solution is null in FVMatAdvectionFunctionBC::computeQpResidual. Did "
      "you suppress the flux_variable_exact_solution parameter in your derived class and forget to "
      "override computeQpResidual?");

  Real flux_var_ghost = _flux_variable_exact_solution->value(
      _t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid());
  RealVectorValue v_ghost(
      _vel_x_exact_solution.value(_t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid()),
      _vel_y_exact_solution ? _vel_y_exact_solution->value(
                                  _t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid())
                            : 0,
      _vel_z_exact_solution ? _vel_z_exact_solution->value(
                                  _t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid())
                            : 0);

  interpolate(Moose::FV::InterpMethod::Average,
              v_face,
              _vel(makeElemArg(&_face_info->elem())),
              v_ghost,
              *_face_info,
              true);

  interpolate(_advected_interp_method,
              flux_var_face,
              _adv_quant(makeElemArg(&_face_info->elem())),
              flux_var_ghost,
              v_face,
              *_face_info,
              true);
  return _normal * v_face * flux_var_face;
}
