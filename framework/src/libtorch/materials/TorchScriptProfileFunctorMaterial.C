//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TorchScriptProfileFunctorMaterial.h"

#ifdef MOOSE_LIBTORCH_ENABLED

#include <cmath>
#include <cstdint>
#include <exception>
#include <set>

registerMooseObject("MooseApp", TorchScriptProfileFunctorMaterial);

InputParameters
TorchScriptProfileFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();

  params.addClassDescription(
      "Evaluates a TorchScript model that returns sampled one-dimensional "
      "profiles and exposes those profiles as interpolated ADReal functors.");

  params.addRequiredParam<UserObjectName>(
      "torch_script_userobject",
      "TorchScriptUserObject containing the deployed TorchScript model.");

  params.addParam<std::vector<PostprocessorName>>(
      "input_names",
      {},
      "Ordered postprocessor values passed to the model. Specify either "
      "input_names or input_values, but not both.");

  params.addParam<std::vector<Real>>(
      "input_values",
      {},
      "Ordered constant values passed directly to the model. Specify either "
      "input_names or input_values, but not both.");

  params.addRequiredParam<std::vector<MooseFunctorName>>(
      "profile_names",
      "Names assigned to model output channels. The number of names must "
      "equal the model output channel count.");

  params.addRequiredParam<std::vector<Real>>(
      "profile_coordinates",
      "Strictly increasing coordinates corresponding to the model output "
      "stations.");

  params.addRequiredParam<Point>(
      "profile_origin",
      "Physical point corresponding to profile coordinate zero.");

  params.addRequiredParam<RealVectorValue>(
      "profile_direction",
      "Physical direction of increasing profile coordinate. The direction "
      "is normalized internally.");

  params.addParam<Real>(
      "coordinate_scale",
      1.0,
      "Physical distance corresponding to one model-coordinate unit.");

  params.addParam<MooseEnum>(
      "out_of_range_behavior",
      MooseEnum("error clamp extrapolate", "error"),
      "Behavior when a MOOSE evaluation point lies outside the supplied "
      "profile coordinate range.");

  params.addParam<MooseEnum>(
      "tensor_dtype",
      MooseEnum("float32 float64", "float64"),
      "Floating-point type expected by the TorchScript model.");

  return params;
}

TorchScriptProfileFunctorMaterial::TorchScriptProfileFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _torch_script_userobject(
        getUserObject<TorchScriptUserObject>("torch_script_userobject")),
    _input_names(
        getParam<std::vector<PostprocessorName>>("input_names")),
    _input_values(
        getParam<std::vector<Real>>("input_values")),
    _profile_names(
        getParam<std::vector<MooseFunctorName>>("profile_names")),
    _profile_coordinates(
        getParam<std::vector<Real>>("profile_coordinates")),
    _profile_origin(
        getParam<Point>("profile_origin")),
    _profile_direction(
        getParam<RealVectorValue>("profile_direction")),
    _coordinate_scale(
        getParam<Real>("coordinate_scale")),
    _out_of_range_behavior(
        getParam<MooseEnum>("out_of_range_behavior")),
    _tensor_dtype(
        getParam<MooseEnum>("tensor_dtype"))
{
  const bool using_postprocessors = !_input_names.empty();
  const bool using_constants = !_input_values.empty();

  if (using_postprocessors == using_constants)
    paramError(
        "input_names",
        "Specify exactly one of input_names or input_values.");

  if (_profile_names.empty())
    paramError(
        "profile_names",
        "At least one output profile name is required.");

  if (_profile_coordinates.empty())
    paramError(
        "profile_coordinates",
        "At least one profile coordinate is required.");

  for (const auto i : index_range(_profile_coordinates))
  {
    if (!std::isfinite(_profile_coordinates[i]))
      paramError(
          "profile_coordinates",
          "All profile coordinates must be finite.");

    if (i > 0 &&
        _profile_coordinates[i] <= _profile_coordinates[i - 1])
      paramError(
          "profile_coordinates",
          "Profile coordinates must be strictly increasing.");
  }

  if (!std::isfinite(_coordinate_scale) ||
      _coordinate_scale <= 0.0)
    paramError(
        "coordinate_scale",
        "coordinate_scale must be finite and greater than zero.");

  const Real direction_norm = _profile_direction.norm();

  if (!std::isfinite(direction_norm) || direction_norm <= 0.0)
    paramError(
        "profile_direction",
        "profile_direction must be finite and nonzero.");

  _profile_direction /= direction_norm;

  if (using_postprocessors)
    for (const auto & input_name : _input_names)
      _postprocessor_inputs.push_back(
          &getPostprocessorValueByName(input_name));

  _profiles.resize(_profile_names.size());

  const std::set<ExecFlagType> clearance_schedule(
      _execute_enum.begin(), _execute_enum.end());

  for (const auto profile_index : index_range(_profile_names))
    addFunctorProperty<ADReal>(
        _profile_names[profile_index],
        [this, profile_index](const auto & spatial_arg,
                              const auto &) -> ADReal
        {
          return sampleProfile(
              profile_index,
              spatial_arg.getPoint());
        },
        clearance_schedule);
}

torch::Tensor
TorchScriptProfileFunctorMaterial::buildInputTensor() const
{
  std::vector<Real> values;

  if (!_input_values.empty())
    values = _input_values;
  else
  {
    values.resize(_postprocessor_inputs.size());

    for (const auto i : index_range(_postprocessor_inputs))
      values[i] = *_postprocessor_inputs[i];
  }

  for (const auto i : index_range(values))
    if (!std::isfinite(values[i]))
      mooseError(
          "TorchScriptProfileFunctorMaterial '",
          name(),
          "' received non-finite model input ",
          i,
          ": ",
          values[i]);

  auto input =
      torch::from_blob(
          values.data(),
          {1, static_cast<std::int64_t>(values.size())},
          torch::TensorOptions()
              .dtype(torch::kFloat64)
              .device(torch::kCPU))
          .clone();

  if (_tensor_dtype == "float32")
    input = input.to(torch::kFloat32);

  return input.to(_app.getLibtorchDevice());
}

void
TorchScriptProfileFunctorMaterial::initialSetup()
{
  torch::Tensor output;

  try
  {
    torch::NoGradGuard no_grad;

    output = _torch_script_userobject
                 .evaluate(buildInputTensor())
                 .detach()
                 .to(torch::kCPU)
                 .to(torch::kFloat64)
                 .contiguous();
  }
  catch (const c10::Error & error)
  {
    mooseError(
        "TorchScript inference failed in ",
        name(),
        ":\n",
        error.what());
  }
  catch (const std::exception & error)
  {
    mooseError(
        "TorchScript inference failed in ",
        name(),
        ":\n",
        error.what());
  }

  /*
   * Normalize supported output layouts to [C, N].
   *
   * [N]       -> [1, N]
   * [C, N]    -> unchanged
   * [1, C, N] -> [C, N]
   */
  if (output.dim() == 1)
  {
    if (_profile_names.size() != 1)
      mooseError(
          "TorchScript model in ",
          name(),
          " returned a one-dimensional tensor, but ",
          _profile_names.size(),
          " profile names were requested.");

    output = output.unsqueeze(0);
  }
  else if (output.dim() == 3)
  {
    if (output.size(0) != 1)
      mooseError(
          "TorchScript model in ",
          name(),
          " returned a three-dimensional tensor whose batch dimension is ",
          output.size(0),
          ". Only a batch size of one is supported.");

    output = output.squeeze(0);
  }

  if (output.dim() != 2)
    mooseError(
        "TorchScript model in ",
        name(),
        " must return [N], [C,N], or [1,C,N]. Received a tensor with ",
        output.dim(),
        " dimensions.");

  const auto expected_channels =
      static_cast<std::int64_t>(_profile_names.size());

  const auto expected_stations =
      static_cast<std::int64_t>(_profile_coordinates.size());

  if (output.size(0) != expected_channels)
    mooseError(
        "TorchScript model in ",
        name(),
        " returned ",
        output.size(0),
        " profile channels, but profile_names contains ",
        expected_channels,
        " entries.");

  if (output.size(1) != expected_stations)
    mooseError(
        "TorchScript model in ",
        name(),
        " returned ",
        output.size(1),
        " stations per profile, but profile_coordinates contains ",
        expected_stations,
        " entries.");

  const auto output_values = output.accessor<double, 2>();

  for (const auto channel : index_range(_profile_names))
  {
    std::vector<Real> values(_profile_coordinates.size());

    for (const auto station : index_range(_profile_coordinates))
    {
      values[station] = output_values[channel][station];

      if (!std::isfinite(values[station]))
        mooseError(
            "TorchScript model in ",
            name(),
            " returned a non-finite value for profile '",
            _profile_names[channel],
            "' at station ",
            station,
            ".");
    }

    const bool extrapolate =
        _out_of_range_behavior == "extrapolate";

    try
    {
      _profiles[channel] =
          std::make_unique<LinearInterpolation>(
              _profile_coordinates,
              values,
              extrapolate);
    }
    catch (const std::exception & error)
    {
      mooseError(
          "Failed to construct profile interpolator '",
          _profile_names[channel],
          "' in ",
          name(),
          ":\n",
          error.what());
    }
  }
}

ADReal
TorchScriptProfileFunctorMaterial::sampleProfile(
    const unsigned int profile_index,
    const Point & point) const
{
  if (profile_index >= _profiles.size())
    mooseError(
        "Invalid profile index ",
        profile_index,
        " requested from ",
        name(),
        ".");

  if (!_profiles[profile_index])
    mooseError(
        "Profile '",
        _profile_names[profile_index],
        "' was evaluated before TorchScript inference completed.");

  const Real coordinate =
      ((point - _profile_origin) * _profile_direction) /
      _coordinate_scale;

  if (!std::isfinite(coordinate))
    mooseError(
        "A non-finite spatial coordinate was calculated while evaluating "
        "profile '",
        _profile_names[profile_index],
        "'.");

  if (_out_of_range_behavior == "error" &&
      (coordinate < _profile_coordinates.front() ||
       coordinate > _profile_coordinates.back()))
    mooseError(
        "Profile coordinate ",
        coordinate,
        " is outside [",
        _profile_coordinates.front(),
        ", ",
        _profile_coordinates.back(),
        "] while evaluating profile '",
        _profile_names[profile_index],
        "' in ",
        name(),
        ".");

  /*
   * LinearInterpolation clamps endpoint values when extrapolation is false.
   * Therefore:
   *
   *   error       handled explicitly above
   *   clamp       extrapolation=false
   *   extrapolate extrapolation=true
   */
  return _profiles[profile_index]->sample(coordinate);
}

#endif
