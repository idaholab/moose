//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVPumpFunctorMaterial.h"
#include "NS.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", NSFVPumpFunctorMaterial);
registerMooseObjectRenamed("NavierStokesApp",
                           NSFVPumpMaterial,
                           "08/01/2024 00:00",
                           NSFVPumpFunctorMaterial);

InputParameters
NSFVPumpFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes the effective pump body force.");
  params.addParam<MooseFunctorName>(
      "pump_force_name", "pump_volume_force", "Name of the pump force functor.");
  params.addParam<FunctionName>("pressure_head_function", "Pressure Head Function.");
  params.addParam<MooseFunctorName>("area_rated", 1.0, "Rated area of the pump.");
  params.addParam<MooseFunctorName>("volume_rated", 1.0, "Rated volume of the pump.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density.");
  params.addRequiredParam<MooseFunctorName>(NS::speed, "Flow speed.");
  params.addParam<Real>("flow_rate_rated", 1.0, "Rated flow rate.");
  params.addParam<PostprocessorName>("flow_rate", 1.0, "Flow rate.");
  params.addParam<RealVectorValue>(
      "gravity", RealVectorValue(0, -9.81, 0), "The gravitational acceleration vector.");
  params.addParam<Real>("rotation_speed_rated", 1.0, "The rated rotation speed of the pump.");
  params.addParam<Real>("rotation_speed", 1.0, "The rotation speed of the pump.");
  params.addParam<bool>(
      "enable_negative_rotation", false, "Flag to allow negative rotation speeds.");
  params.addParam<bool>("symmetric_negative_pressure_head",
                        true,
                        "Flag to use the pressure head function in the negative direction than the "
                        "one in the positive direction.");
  params.addParam<FunctionName>("pressure_head_function_negative_rotation",
                                "Pressure head function for negative rotation.");
  params.declareControllable("rotation_speed");
  return params;
}

NSFVPumpFunctorMaterial::NSFVPumpFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _pressure_head_function(
        isParamValid("pressure_head_function") ? &getFunction("pressure_head_function") : nullptr),
    _area_rated(getFunctor<Real>("area_rated")),
    _volume_rated(getFunctor<Real>("volume_rated")),
    _rho(getFunctor<ADReal>(NS::density)),
    _speed(getFunctor<ADReal>(NS::speed)),
    _flow_rate_rated(getParam<Real>("flow_rate_rated")),
    _flow_rate(getPostprocessorValue("flow_rate")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _rotation_speed_rated(getParam<Real>("rotation_speed_rated")),
    _rotation_speed(getParam<Real>("rotation_speed")),
    _flow_rate_scaling_bool(isParamSetByUser("flow_rate_rated")),
    _bool_negative_rotation_speed(getParam<bool>("enable_negative_rotation")),
    _bool_symmetric_negative_pressure_head(getParam<bool>("symmetric_negative_pressure_head")),
    _pressure_head_function_negative_rotation(
        isParamValid("pressure_head_function_negative_rotation")
            ? &getFunction("pressure_head_function_negative_rotation")
            : nullptr)
{
  // Error checks
  if (!_pressure_head_function && !_pressure_head_function_negative_rotation)
    paramError("pressure_head_function",
               "Pressure head function should be provided. If negative rotation is used "
               "'pressure_head_function_negative_rotation' should be provided.");

  if (!_bool_negative_rotation_speed && _rotation_speed < 0)
    paramError("rotation_speed",
               "The rotation speed must be positive if 'enable_negative_rotation' is not true.");

  if (!_bool_negative_rotation_speed && _pressure_head_function_negative_rotation)
    paramError(
        "pressure_head_function_negative_rotation",
        "The negative pressure head won't be used if 'enable_negative_rotation' is not true.");

  if (!_bool_symmetric_negative_pressure_head && !_pressure_head_function_negative_rotation)
    paramError("pressure_head_function_negative_rotation",
               "Negative pressure head function should be provided if "
               "'symmetric_negative_pressure_head' is false.");

  addFunctorProperty<Real>(
      getParam<MooseFunctorName>("pump_force_name"),
      [this](const auto & r, const auto & t) -> Real
      {
        // Getting rated pressure head
        const Point ref_pressure_point(0.0, 0.0, 0.0);
        Real rated_pressure_head;
        if (_bool_symmetric_negative_pressure_head)
          rated_pressure_head =
              (*_pressure_head_function).value(std::abs(_flow_rate), ref_pressure_point);
        else
          rated_pressure_head = -(*_pressure_head_function_negative_rotation)
                                     .value(std::abs(_flow_rate), ref_pressure_point);

        // Scaling pressure head
        const auto flow_rate_scaling =
            _flow_rate_scaling_bool ? std::sqrt(std::abs(_flow_rate) / _flow_rate_rated) : 1.0;
        const auto rotation_speed_scaling = std::abs(_rotation_speed) / _rotation_speed_rated;
        const auto pressure_head =
            rated_pressure_head * std::pow(flow_rate_scaling * rotation_speed_scaling, 4.0 / 3.0);

        // Computing effective volume force
        mooseAssert(_rho.isConstant(),
                    "The density must be a constant in order for the pump force to not contain "
                    "derivative information.");
        const auto rho = raw_value(_rho(r, t));
        const Real gravity = _gravity.norm();
        const auto area = _area_rated(r, t);
        const auto volume = _volume_rated(r, t);
        return -rho * gravity * pressure_head * area / volume;
      });

  const auto & mesh_blocks = _subproblem.mesh().meshSubdomains();
  const auto & pump_blocks = blockIDs();
  std::set<SubdomainID> missing_blocks;

  std::set_difference(mesh_blocks.begin(),
                      mesh_blocks.end(),
                      pump_blocks.begin(),
                      pump_blocks.end(),
                      std::inserter(missing_blocks, missing_blocks.begin()));

  if (missing_blocks.size() >= 1 && *(pump_blocks.begin()) != Moose::ANY_BLOCK_ID)
  {
    _subproblem.addPiecewiseByBlockLambdaFunctor<Real>(
        getParam<MooseFunctorName>("pump_force_name"),
        [](const auto &, const auto &) -> Real { return 0.0; },
        std::set<ExecFlagType>({EXEC_ALWAYS}),
        _subproblem.mesh(),
        missing_blocks,
        _tid);
  }
}
