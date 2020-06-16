//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADLAROMANCEStressUpdateBase.h"

#include "Function.h"
#include "MathUtils.h"

InputParameters
ADLAROMANCEStressUpdateBase::validParams()
{
  InputParameters params = ADRadialReturnCreepStressUpdateBase::validParams();
  params.addClassDescription(
      "Calculates the effective creep strain based on the rates predicted by a material "
      "specific Los Alamos Reduced Order Model derived from a Visco-Plastic Self Consistent "
      "calculations.");

  params.addRequiredCoupledVar("temperature", "The coupled temperature (K)");
  params.addParam<MaterialPropertyName>("environmental_factor",
                                        "Optional coupled environmental factor");
  MooseEnum window_failure_extrapolate("ERROR WARN IGNORE EXTRAPOLATE", "EXTRAPOLATE");
  MooseEnum window_failure_error("ERROR WARN IGNORE EXTRAPOLATE", "ERROR");
  params.addParam<MooseEnum>("mobile_input_window_failure",
                             window_failure_error,
                             "What to do if mobile dislocation concentration is outside the global "
                             "window of applicability.");
  params.addParam<MooseEnum>("immobile_input_window_failure",
                             window_failure_error,
                             "What to do if immobile dislocation concentration is outside the "
                             "global window of applicability.");
  params.addParam<MooseEnum>("stress_input_window_failure",
                             window_failure_extrapolate,
                             "What to do if stress is outside the global window of applicability.");
  params.addParam<MooseEnum>(
      "old_strain_input_window_failure",
      window_failure_error,
      "What to do if old strain is outside the global window of applicability.");
  params.addParam<MooseEnum>(
      "temperature_input_window_failure",
      window_failure_extrapolate,
      "What to do if temperature is outside the global window of applicability.");
  params.addParam<MooseEnum>(
      "environment_input_window_failure",
      window_failure_extrapolate,
      "What to do if environmental factor is outside the global window of applicability.");

  params.addRangeCheckedParam<Real>("initial_mobile_dislocation_density",
                                    0.0,
                                    "initial_mobile_dislocation_density >= 0.0",
                                    "Initial density of mobile (glissile) dislocations (1/m^2)");
  params.addRangeCheckedParam<Real>(
      "max_relative_mobile_dislocation_increment",
      0.5,
      "max_relative_mobile_dislocation_increment > 0.0",
      "Maximum increment of density of mobile (glissile) dislocations.");
  params.addParam<FunctionName>(
      "mobile_dislocation_density_forcing_function",
      "Optional forcing function for immobile dislocation. If provided, the immobile dislocation "
      "density will be reset to the function value at the beginning of the timestep. Used for "
      "testing purposes only.");
  params.addRangeCheckedParam<Real>("initial_immobile_dislocation_density",
                                    0.0,
                                    "initial_immobile_dislocation_density >= 0.0",
                                    "Immobile (locked) dislocation density initial value (1/m^2).");
  params.addRangeCheckedParam<Real>(
      "max_relative_immobile_dislocation_increment",
      0.5,
      "max_relative_immobile_dislocation_increment > 0.0",
      "Maximum increment of immobile (locked) dislocation density initial value (1/m^2).");
  params.addParam<FunctionName>(
      "immobile_dislocation_density_forcing_function",
      "Optional forcing function for immobile dislocation. If provided, the immobile dislocation "
      "density will be reset to the function value at the beginning of the timestep. Used for "
      "testing purposes only.");

  params.addParam<FunctionName>(
      "old_creep_strain_forcing_function",
      "Optional forcing function for the creep strain from the previous timestep. If provided, "
      "the old creep strain will be reset to the function value at the beginning of the "
      "timestep. Used for testing purposes only.");

  params.addParam<bool>("verbose", false, "Flag to add verbose output");

  params.addParamNamesToGroup(
      "mobile_dislocation_density_forcing_function immobile_dislocation_density_forcing_function "
      "old_creep_strain_forcing_function",
      "Advanced");
  return params;
}

ADLAROMANCEStressUpdateBase::ADLAROMANCEStressUpdateBase(const InputParameters & parameters)
  : ADRadialReturnCreepStressUpdateBase(parameters),
    _temperature(adCoupledValue("temperature")),
    _environmental(isParamValid("environmental_factor")
                       ? &getADMaterialProperty<Real>("environmental_factor")
                       : nullptr),
    _window_failure(6),

    _verbose(getParam<bool>("verbose")),
    _mobile_dislocations(declareADProperty<Real>(_base_name + "mobile_dislocations")),
    _mobile_dislocations_old(getMaterialPropertyOld<Real>(_base_name + "mobile_dislocations")),
    _initial_mobile_dislocations(getParam<Real>("initial_mobile_dislocation_density")),
    _max_mobile_increment(getParam<Real>("max_relative_mobile_dislocation_increment")),
    _mobile_function(isParamValid("mobile_dislocation_density_forcing_function")
                         ? &getFunction("mobile_dislocation_density_forcing_function")
                         : NULL),
    _mobile_dislocation_increment(0.0),
    _mobile_old(0.0),
    _immobile_dislocations(declareADProperty<Real>(_base_name + "immobile_dislocations")),
    _immobile_dislocations_old(getMaterialPropertyOld<Real>(_base_name + "immobile_dislocations")),
    _initial_immobile_dislocations(getParam<Real>("initial_immobile_dislocation_density")),
    _max_immobile_increment(getParam<Real>("max_relative_immobile_dislocation_increment")),
    _immobile_function(isParamValid("immobile_dislocation_density_forcing_function")
                           ? &getFunction("immobile_dislocation_density_forcing_function")
                           : NULL),
    _immobile_dislocation_increment(0.0),
    _immobile_old(0.0),
    _effective_strain_old(0.0),
    _mobile_input_idx(0),
    _immobile_input_idx(1),
    _stress_input_idx(2),
    _old_strain_input_idx(3),
    _temperature_input_idx(4),
    _environmental_input_idx(5),
    _mobile_output_idx(0),
    _immobile_output_idx(1),
    _strain_output_idx(2),

    _creep_strain_old_forcing_function(isParamValid("old_creep_strain_forcing_function")
                                           ? &getFunction("old_creep_strain_forcing_function")
                                           : NULL),

    _creep_rate(declareADProperty<Real>(_base_name + "creep_rate")),
    _mobile_rate(declareADProperty<Real>(_base_name + "mobile_dislocation_rate")),
    _immobile_rate(declareADProperty<Real>(_base_name + "immobile_dislocation_rate")),
    _extrapolation(declareADProperty<Real>("ROM_extrapolation")),

    _derivative(0.0),
    _old_input_values(3)
{
  _check_range = true;

  _window_failure[_mobile_input_idx] =
      parameters.get<MooseEnum>("mobile_input_window_failure").getEnum<WindowFailure>();
  _window_failure[_immobile_input_idx] =
      parameters.get<MooseEnum>("immobile_input_window_failure").getEnum<WindowFailure>();
  _window_failure[_stress_input_idx] =
      parameters.get<MooseEnum>("stress_input_window_failure").getEnum<WindowFailure>();
  _window_failure[_old_strain_input_idx] =
      parameters.get<MooseEnum>("old_strain_input_window_failure").getEnum<WindowFailure>();
  _window_failure[_temperature_input_idx] =
      parameters.get<MooseEnum>("temperature_input_window_failure").getEnum<WindowFailure>();
  _window_failure[_environmental_input_idx] =
      parameters.get<MooseEnum>("environment_input_window_failure").getEnum<WindowFailure>();
}

void
ADLAROMANCEStressUpdateBase::initialSetup()
{
  // Pull in relevant ROM information and do sanity checks
  _transform = getTransform();
  _transform_coefs = getTransformCoefs();
  _input_limits = getInputLimits();
  _coefs = getCoefs();
  _tiling = getTilings();

  _num_tiles = _transform.size();
  if (!_num_tiles)
    mooseError("In ", _name, ": No tiles detected. Double check your ROM input");

  _num_outputs = _transform[0].size();
  if (_num_outputs != 3)
    mooseError("In ",
               _name,
               ": ",
               _num_outputs,
               " outputs detected. Three and only three outputs are currently supported.");

  _num_inputs = _transform[0][0].size();
  if (_num_inputs != 5 && _num_inputs != 6)
    mooseError("In ",
               _name,
               ": ",
               _num_inputs,
               " inputs detected. Only five or six inputs currently supported.");
  if (_num_inputs == 6 && !_environmental)
    paramError("environmental",
               "Number of ROM inputs indicate environmental factor is required to be coupled.");
  if (_num_inputs != 6 && _environmental)
    paramError("environmental",
               "Number of ROM inputs indicate environmental factor is not implemented, but "
               "environmental factor coupled.");

  bool correct_shape = true;
  if (_transform.size() != _num_tiles || _transform_coefs.size() != _num_tiles ||
      _input_limits.size() != _num_tiles || _coefs.size() != _num_tiles)
    correct_shape = false;
  if (_tiling.size() != _num_inputs)
    correct_shape = false;

  if (_coefs[0].size() == 0)
    correct_shape = false;
  _num_coefs = _coefs[0][0].size();
  for (unsigned int t = 0; t < _num_tiles; ++t)
  {
    if (_transform[t].size() != _num_outputs || _transform_coefs[t].size() != _num_outputs ||
        _coefs[t].size() != _num_outputs)
      correct_shape = false;
    for (unsigned int o = 0; o < _num_outputs; ++o)
      if (_transform[t][o].size() != _num_inputs || _transform_coefs[t][o].size() != _num_inputs ||
          _coefs[t][o].size() != _num_coefs)
        correct_shape = false;
    if (_input_limits[t].size() != _num_inputs)
      correct_shape = false;
    for (unsigned int i = 0; i < _num_inputs; ++i)
      if (_input_limits[t][i].size() != 2)
        correct_shape = false;
  }

  if (!correct_shape)
    mooseError("In ", _name, ": ROM data is not the right shape.");

  unsigned int tiles_check = 1;
  for (unsigned int i = 0; i < _num_inputs; ++i)
    tiles_check *= _tiling[i];
  if (tiles_check != _num_tiles)
    mooseError("In ",
               _name,
               ": Product of getTiles (",
               tiles_check,
               ") is not the same as num_tiles (",
               _num_tiles,
               ")");

  _degree = std::pow(_num_coefs, 1.0 / _num_inputs);
  if (!_degree || _degree > 4)
    mooseError("In ", _name, ": degree must be 1, 2, 3 or 4.");

  // Check input limits and find global limits
  _global_limits.resize(_num_inputs, std::vector<Real>(2));
  for (unsigned int i = 0; i < _num_inputs; ++i)
  {
    std::vector<Real> all_lows;
    std::vector<Real> all_highs;
    for (unsigned int t = 0; t < _num_tiles; ++t)
    {
      if (std::find(all_lows.begin(), all_lows.end(), _input_limits[t][i][0]) == all_lows.end())
        all_lows.push_back(_input_limits[t][i][0]);
      if (std::find(all_highs.begin(), all_highs.end(), _input_limits[t][i][1]) == all_highs.end())
        all_highs.push_back(_input_limits[t][i][1]);
    }
    // todo more checks

    _global_limits[i][0] = all_lows.front();
    _global_limits[i][1] = all_highs.back();
  }

  /// Check for divide potential divide by zero when applying EXP ROM transform
  for (unsigned int t = 0; t < _num_tiles; ++t)
  {
    for (unsigned int o = 0; o < _num_outputs; ++o)
    {
      for (unsigned int i = 0; i < _num_inputs; ++i)
      {
        if (_transform[t][o][i] == ROMInputTransform::EXP && _transform_coefs[t][o][i] == 0)
          mooseError("In ",
                     _name,
                     ": One of the transform coefficients is 0 with an exponential transform. This "
                     "will lead to a divide-by-zero error. Check the ROM parameters");
        if (_transform[t][o][i] == ROMInputTransform::LINEAR && _transform_coefs[t][o][i] != 0)
          mooseError("In ",
                     _name,
                     ": One of the transform coefficients is not 0 with an linear transform.");
      }
    }
  }

  // Precompute helper containers
  _transformed_limits = getTransformedLimits();
  _makeframe_helper = getMakeFrameHelper();

  // Prepare containers
  _input_values.resize(_num_inputs);
  _precomputed_vals.resize(_num_tiles, std::vector<ADReal>(_num_coefs));
  _rom_inputs.resize(_num_tiles, std::vector<ADReal>(_num_inputs));
  _polynomial_inputs.resize(
      _num_tiles, std::vector<std::vector<ADReal>>(_num_inputs, std::vector<ADReal>(_degree)));
  _non_stress_weights.resize(_num_tiles);
  _weights.resize(_num_tiles);

  if (_verbose)
  {
    Moose::err << "ROM model info: " << _name << "\n";
    Moose::err << " number of tiles: " << _num_tiles << "\n";
    Moose::err << " number of outputs: " << _num_outputs << "\n";
    Moose::err << " number of inputs: " << _num_inputs << "\n";
    Moose::err << " degree (max Legendre degree + constant): " << _degree << "\n";
    Moose::err << " number of coefficients: " << _num_coefs << "\n";
    Moose::err << " Global limits:\n  mobile dislocations (" << _global_limits[_mobile_input_idx][0]
               << " - " << _global_limits[_mobile_input_idx][1] << ")\n";
    Moose::err << "  immobile dislocations (" << _global_limits[_immobile_input_idx][0] << " - "
               << _global_limits[_immobile_input_idx][1] << ")\n";
    Moose::err << "  Stress (" << _global_limits[_stress_input_idx][0] << " - "
               << _global_limits[_stress_input_idx][1] << ")\n";
    Moose::err << "  Old strain (" << _global_limits[_old_strain_input_idx][0] << " - "
               << _global_limits[_old_strain_input_idx][1] << ")\n";
    Moose::err << "  Temperature (" << _global_limits[_temperature_input_idx][0] << " - "
               << _global_limits[_temperature_input_idx][1] << ")\n";
    if (_environmental)
      Moose::err << "  Environmental factor (" << _global_limits[_environmental_input_idx][0]
                 << " - " << _global_limits[_environmental_input_idx][1] << ")\n";
    Moose::err << std::endl;
  }
}

void
ADLAROMANCEStressUpdateBase::initQpStatefulProperties()
{
  _mobile_dislocations[_qp] = _initial_mobile_dislocations;
  _immobile_dislocations[_qp] = _initial_immobile_dislocations;

  ADRadialReturnCreepStressUpdateBase::initQpStatefulProperties();
}

ADReal
ADLAROMANCEStressUpdateBase::maximumPermissibleValue(const ADReal & effective_trial_stress) const
{
  // Make maximum allowed scalar a little bit less than the deformation that would reduce the
  // trial stress to zero. This prevents negative trial stresses.
  return effective_trial_stress / _three_shear_modulus * 0.999999;
}

void
ADLAROMANCEStressUpdateBase::computeStressInitialize(const ADReal & effective_trial_stress,
                                         const ADRankFourTensor & elasticity_tensor)
{
  ADRadialReturnCreepStressUpdateBase::computeStressInitialize(effective_trial_stress,
                                                               elasticity_tensor);

  _immobile_old = _immobile_function ? _immobile_function->value(_t, _q_point[_qp])
                                     : _immobile_dislocations_old[_qp];
  _mobile_old =
      _mobile_function ? _mobile_function->value(_t, _q_point[_qp]) : _mobile_dislocations_old[_qp];
  _effective_strain_old =
      _creep_strain_old_forcing_function
          ? _creep_strain_old_forcing_function->value(_t, _q_point[_qp])
          : std::sqrt(_creep_strain_old[_qp].doubleContraction(_creep_strain_old[_qp]) / 1.5);

  // Prepare input
  _input_values[_mobile_input_idx] = _mobile_old;
  _input_values[_immobile_input_idx] = _immobile_old;
  _input_values[_stress_input_idx] = effective_trial_stress * 1.0e-6;
  _input_values[_old_strain_input_idx] = _effective_strain_old;
  _input_values[_temperature_input_idx] = _temperature[_qp];
  if (_environmental)
    _input_values[_environmental_input_idx] = (*_environmental)[_qp];

  _old_input_values[_mobile_output_idx] = _mobile_old;
  _old_input_values[_immobile_output_idx] = _immobile_old;
  _old_input_values[_strain_output_idx] = _effective_strain_old;

  // Determine tile mixing weight
  std::fill(_non_stress_weights.begin(), _non_stress_weights.end(), 1.0);
  for (unsigned int i = 0; i < _num_inputs; i++)
    if (i != _stress_input_idx)
      computeTileWeight(_non_stress_weights, _input_values[i], i);

  // Check to see if input is in range
  for (unsigned int i = 0; i < _num_inputs; i++)
    if (i != _stress_input_idx)
      checkInputWindow(_input_values[i], _window_failure[i], _global_limits[i]);

  // Precompute transformed input and prebuild polynomials for inputs other than strain
  precomputeROM(_strain_output_idx);
}

void
ADLAROMANCEStressUpdateBase::computeTileWeight(std::vector<ADReal> & weights,
                                   ADReal & input,
                                   const unsigned int in_idx,
                                   const bool derivative)
{
  bool extrapolated = false;
  for (unsigned int t = 0; t < _num_tiles; ++t)
  {
    if (_tiling[in_idx] < 2)
    {
      if (_window_failure[in_idx] == WindowFailure::EXTRAPOLATE &&
          _input_limits[t][in_idx][0] == _global_limits[in_idx][0] &&
          input < _input_limits[t][in_idx][0])
      {
        extrapolated = true;
        if (derivative)
          weights[t] *= -sigmoid(0.0, _global_limits[in_idx][0], input, true);
        else
          weights[t] *= (1.0 - sigmoid(0.0, _global_limits[in_idx][0], input));
      }
    }
    else
    {
      if (input >= _input_limits[t][in_idx][0] && input < _input_limits[t][in_idx][1])
      {
        bool overlap = false;
        for (unsigned int tt = 0; tt < _num_tiles; ++tt)
        {
          if (!overlap && t != tt)
          {
            if ((_input_limits[t][in_idx][0] != _input_limits[tt][in_idx][0] ||
                 _input_limits[t][in_idx][1] != _input_limits[tt][in_idx][1]) &&
                input >= _input_limits[tt][in_idx][0] && input < _input_limits[tt][in_idx][1])
            {
              if (_input_limits[t][in_idx][0] < _input_limits[tt][in_idx][0] &&
                  _input_limits[t][in_idx][1] > _input_limits[tt][in_idx][0])
              {
                weights[t] *= sigmoid(
                    _input_limits[tt][in_idx][0], _input_limits[t][in_idx][1], input, derivative);
              }
              else if (_input_limits[t][in_idx][0] > _input_limits[tt][in_idx][0] &&
                       _input_limits[t][in_idx][0] < _input_limits[tt][in_idx][1])
              {
                if (derivative)
                  weights[t] *= -sigmoid(
                      _input_limits[t][in_idx][0], _input_limits[tt][in_idx][1], input, derivative);
                else
                  weights[t] *=
                      (1.0 -
                       sigmoid(_input_limits[t][in_idx][0], _input_limits[tt][in_idx][1], input));
              }
              overlap = true;
            }
          }
        }
        if (!overlap && derivative)
          weights[t] *= 0.0;
      }
      else
        weights[t] *= 0.0;
    }
  }
  if (extrapolated)
    input = _global_limits[in_idx][0];
}

ADReal
ADLAROMANCEStressUpdateBase::computeResidual(const ADReal & effective_trial_stress, const ADReal & scalar)
{
  // Update new stress
  ADReal trial_stress_mpa = effective_trial_stress * 1.0e-6;
  ADReal dtrial_stress_dscalar = 0.0;
  if (_apply_strain)
  {
    trial_stress_mpa -= _three_shear_modulus * scalar * 1.0e-6;
    dtrial_stress_dscalar -= _three_shear_modulus * 1.0e-6;
  }
  _input_values[_stress_input_idx] = trial_stress_mpa;

  // Update weights with new stress
  _weights = _non_stress_weights;
  auto dweights_dstress = _non_stress_weights;
  computeTileWeight(_weights, _input_values[_stress_input_idx], _stress_input_idx);
  computeTileWeight(dweights_dstress, _input_values[_stress_input_idx], _stress_input_idx, true);

  // Check window with new stress
  checkInputWindow(_input_values[_stress_input_idx],
                   _window_failure[_stress_input_idx],
                   _global_limits[_stress_input_idx]);

  _extrapolation[_qp] = 0.0;
  for (unsigned int t = 0; t < _num_tiles; ++t)
    _extrapolation[_qp] += _weights[t];

  ADReal total_rom_effective_strain_inc = 0.0;
  ADReal dtotal_rom_effective_strain_inc_dstress = 0.0;

  // Run ROM if all values are within windows.
  for (unsigned int t = 0; t < _num_tiles; ++t)
  {
    if (_weights[t])
    {
      const ADReal rom = computeROM(t, _strain_output_idx);
      total_rom_effective_strain_inc += _weights[t] * rom;
      dtotal_rom_effective_strain_inc_dstress +=
          _weights[t] * computeROM(t, _strain_output_idx, true);
      dtotal_rom_effective_strain_inc_dstress += dweights_dstress[t] * rom;
    }
  }

  if (_verbose)
  {
    ADReal environmental = 0.0;
    if (_environmental)
      environmental = (*_environmental)[_qp];
    Moose::err << "Verbose information from " << _name << ": \n";
    Moose::err << "  dt: " << _dt << "\n";
    Moose::err << "  old mobile disl: " << _mobile_old << "\n";
    Moose::err << "  old immobile disl: " << _immobile_old << "\n";
    Moose::err << "  initial stress (MPa): "
               << MetaPhysicL::raw_value(effective_trial_stress) * 1.0e-6 << "\n";
    Moose::err << "  temperature: " << MetaPhysicL::raw_value(_temperature[_qp]) << "\n";
    Moose::err << "  environmental factor: " << MetaPhysicL::raw_value(environmental) << "\n";
    Moose::err << "  calculated scalar strain value: " << MetaPhysicL::raw_value(scalar) << "\n";
    Moose::err << "  trial stress into rom (MPa): " << MetaPhysicL::raw_value(trial_stress_mpa)
               << "\n";
    Moose::err << "  old effective strain: " << _effective_strain_old << "\n";
    Moose::err << "  extrapolation: " << MetaPhysicL::raw_value(_extrapolation[_qp]) << "\n";
    Moose::err << "  weights by tile: ";
    for (unsigned int t = 0; t < _num_tiles; ++t)
      Moose::err << " (" << t << ", " << MetaPhysicL::raw_value(_weights[t]) << ") ";
    Moose::err << "\n";
    Moose::err << "  nonstress weights by tile: ";
    for (unsigned int t = 0; t < _num_tiles; ++t)
      Moose::err << " (" << t << ", " << MetaPhysicL::raw_value(_non_stress_weights[t]) << ") ";
    Moose::err << "\n";
    Moose::err << "  effective strain increment: "
               << MetaPhysicL::raw_value(total_rom_effective_strain_inc) << std::endl;
  }

  // Print out relevant verbose information, and quit creep stress calculation
  if (!_extrapolation[_qp])
  {
    if (scalar)
      mooseError("In ", _name, ": Internal error: Scalar (", scalar, ") isn't zero!");
    if (_verbose)
      Moose::err << "    ROM evalulation skipped due to out-of-range input!" << std::endl;

    _creep_rate[_qp] = 0.0;
    _derivative = 1.0;
    return 0.0;
  }

  _creep_rate[_qp] = total_rom_effective_strain_inc / _dt;
  _derivative = dtotal_rom_effective_strain_inc_dstress * dtrial_stress_dscalar - 1.0;

  if (!_apply_strain)
  {
    if (_verbose)
      Moose::err << "    Strain not applied due to apply_strain input parameter!" << std::endl;
    _derivative = 1.0;
    return 0.0;
  }
  return total_rom_effective_strain_inc - scalar;
}

void
ADLAROMANCEStressUpdateBase::precomputeROM(const unsigned out_idx)
{
  for (unsigned int t = 0; t < _num_tiles; ++t)
  {
    if (_non_stress_weights[t])
    {
      for (unsigned int i = 0; i < _num_inputs; ++i)
      {
        if (i != _stress_input_idx)
        {
          _rom_inputs[t][i] = normalizeInput(_input_values[i],
                                             _transform[t][out_idx][i],
                                             _transform_coefs[t][out_idx][i],
                                             _transformed_limits[t][out_idx][i]);
          buildPolynomials(_rom_inputs[t][i], _polynomial_inputs[t][i]);
        }
      }
      precomputeValues(_coefs[t][out_idx], _polynomial_inputs[t], _precomputed_vals[t]);
    }
  }
}

ADReal
ADLAROMANCEStressUpdateBase::computeROM(const unsigned int t, const unsigned out_idx, const bool derivative)
{
  // Update due to new stress
  _rom_inputs[t][_stress_input_idx] =
      normalizeInput(_input_values[_stress_input_idx],
                     _transform[t][out_idx][_stress_input_idx],
                     _transform_coefs[t][out_idx][_stress_input_idx],
                     _transformed_limits[t][out_idx][_stress_input_idx]);
  buildPolynomials(_rom_inputs[t][_stress_input_idx], _polynomial_inputs[t][_stress_input_idx]);

  // Compute ROM values
  const ADReal rom_output = computeValues(_precomputed_vals[t], _polynomial_inputs[t]);

  // Return converted output if not derivative
  if (!derivative)
    return convertOutput(_old_input_values, rom_output, out_idx);

  const ADReal drom_input = normalizeInput(_input_values[_stress_input_idx],
                                           _transform[t][out_idx][_stress_input_idx],
                                           _transform_coefs[t][out_idx][_stress_input_idx],
                                           _transformed_limits[t][out_idx][_stress_input_idx],
                                           derivative);

  std::vector<ADReal> dpolynomial_inputs(_degree, 0.0);
  buildPolynomials(_rom_inputs[t][_stress_input_idx], dpolynomial_inputs, drom_input, derivative);

  const ADReal drom_output =
      computeValues(_precomputed_vals[t], _polynomial_inputs[t], dpolynomial_inputs, derivative);

  return convertOutput(_old_input_values, rom_output, out_idx, drom_output, derivative);
}

void
ADLAROMANCEStressUpdateBase::checkInputWindow(const ADReal & input,
                                  const WindowFailure behavior,
                                  const std::vector<Real> & global_limits)
{
  if (behavior != WindowFailure::WARN || behavior != WindowFailure::ERROR)
    return;

  if (input < global_limits[0] || input > global_limits[1])
  {
    std::stringstream msg;
    msg << "In " << _name << ": input parameter with value (" << MetaPhysicL::raw_value(input)
        << ") is out of global range (" << global_limits[0] << " - " << global_limits[1] << ")";

    if (behavior == WindowFailure::WARN)
      mooseWarning(msg.str());
    else if (behavior == WindowFailure::ERROR)
      mooseError(msg.str());
  }
}

ADReal
ADLAROMANCEStressUpdateBase::normalizeInput(const ADReal & input,
                                const ROMInputTransform transform,
                                const Real transform_coef,
                                const std::vector<Real> & transformed_limits,
                                const bool derivative)
{
  ADReal x = input;
  convertValue(x, transform, transform_coef, derivative);

  if (derivative)
    return x / transformed_limits[2];
  else
    return (x - transformed_limits[0]) / transformed_limits[2] - 1.0;
}

void
ADLAROMANCEStressUpdateBase::buildPolynomials(const ADReal & rom_input,
                                  std::vector<ADReal> & polynomial_inputs,
                                  const ADReal & drom_input,
                                  const bool derivative)
{
  for (unsigned int d = 0; d < _degree; ++d)
  {
    polynomial_inputs[d] = computePolynomial(rom_input, d);
    if (derivative)
      polynomial_inputs[d] =
          drom_input * computePolynomial(rom_input, d, derivative) / polynomial_inputs[d];
  }
}

void
ADLAROMANCEStressUpdateBase::precomputeValues(const std::vector<Real> & coefs,
                                  const std::vector<std::vector<ADReal>> & polynomial_inputs,
                                  std::vector<ADReal> & precomputed)
{
  for (unsigned int c = 0; c < _num_coefs; ++c)
  {
    precomputed[c] = coefs[c];
    for (unsigned int i = 0; i < _num_inputs; ++i)
      if (i != _stress_input_idx)
        precomputed[c] *= polynomial_inputs[i][_makeframe_helper[c + _num_coefs * i]];
  }
}

ADReal
ADLAROMANCEStressUpdateBase::computeValues(const std::vector<ADReal> & precomputed,
                               const std::vector<std::vector<ADReal>> & polynomial_inputs,
                               const std::vector<ADReal> & dpolynomial_inputs,
                               const bool derivative)
{
  ADReal rom_output = 0.0;
  ADReal val;
  for (unsigned int j = 0; j < _num_coefs; ++j)
  {
    val =
        precomputed[j] *
        polynomial_inputs[_stress_input_idx][_makeframe_helper[j + _num_coefs * _stress_input_idx]];

    if (!derivative)
      rom_output += val;
    else
      rom_output += dpolynomial_inputs[_makeframe_helper[j + _num_coefs * _stress_input_idx]] * val;
  }
  return rom_output;
}

ADReal
ADLAROMANCEStressUpdateBase::convertOutput(const std::vector<Real> & old_input_values,
                               const ADReal & rom_output,
                               const unsigned out_idx,
                               const ADReal & drom_output,
                               const bool derivative)
{
  if (out_idx == _strain_output_idx)
  {
    if (derivative)
      return std::exp(rom_output) * _dt * drom_output;
    else
      return std::exp(rom_output) * _dt;
  }

  if (derivative)
    return 0.0;

  ADReal expout = std::exp(rom_output);
  if (expout > 1.0e-10)
    expout -= 1.000000E-10;
  else
    expout = -1.0E-10 * 1.0E-10 / expout + 1.0E-10;

  return -expout * old_input_values[out_idx] * _dt;
}

ADReal
ADLAROMANCEStressUpdateBase::computePolynomial(const ADReal & value,
                                   const unsigned int degree,
                                   const bool derivative)
{
  if (degree == 0)
  {
    if (derivative)
      return 0.0;
    return 1.0;
  }
  else if (degree == 1)
  {
    if (derivative)
      return 1.0;
    return value;
  }
  else if (degree == 2)
  {
    if (derivative)
      return 3.0 * value;
    return 1.5 * Utility::pow<2>(value) - 0.5;
  }
  else
  {
    if (derivative)
      return 7.5 * Utility::pow<2>(value) - 1.5;
    return 2.5 * Utility::pow<3>(value) - 1.5 * value;
  }
}

std::vector<std::vector<std::vector<std::vector<Real>>>>
ADLAROMANCEStressUpdateBase::getTransformedLimits()
{
  std::vector<std::vector<std::vector<std::vector<Real>>>> transformed_limits(
      _num_tiles,
      std::vector<std::vector<std::vector<Real>>>(
          _num_outputs, std::vector<std::vector<Real>>(_num_inputs, std::vector<Real>(3))));

  for (unsigned int t = 0; t < _num_tiles; ++t)
  {
    for (unsigned int i = 0; i < _num_outputs; ++i)
    {
      transformed_limits[t][i] = _input_limits[t];
      for (unsigned int j = 0; j < _num_inputs; ++j)
      {
        for (unsigned int k = 0; k < 2; ++k)
          convertValue(
              transformed_limits[t][i][j][k], _transform[t][i][j], _transform_coefs[t][i][j]);
        transformed_limits[t][i][j][2] =
            (transformed_limits[t][i][j][1] - transformed_limits[t][i][j][0]) / 2.0;
      }
    }
  }

  return transformed_limits;
}

std::vector<unsigned int>
ADLAROMANCEStressUpdateBase::getMakeFrameHelper() const
{
  std::vector<unsigned int> v(_num_coefs * _num_inputs);

  for (unsigned int numcoeffs = 0; numcoeffs < _num_coefs; ++numcoeffs)
    for (unsigned int invar = 0; invar < _num_inputs; ++invar)
      v[numcoeffs + _num_coefs * invar] = numcoeffs / MathUtils::pow(_degree, invar) % _degree;

  return v;
}

void
ADLAROMANCEStressUpdateBase::computeStressFinalize(const ADRankTwoTensor & plastic_strain_increment)
{
  _mobile_dislocation_increment = 0.0;
  _immobile_dislocation_increment = 0.0;

  precomputeROM(_mobile_output_idx);
  for (unsigned int t = 0; t < _num_tiles; ++t)
    if (_weights[t])
      _mobile_dislocation_increment += _weights[t] * computeROM(t, _mobile_output_idx);

  precomputeROM(_immobile_output_idx);
  for (unsigned int t = 0; t < _num_tiles; ++t)
    if (_weights[t])
      _immobile_dislocation_increment += _weights[t] * computeROM(t, _immobile_output_idx);

  _mobile_rate[_qp] = _mobile_dislocation_increment / _dt;
  _immobile_rate[_qp] = _immobile_dislocation_increment / _dt;
  _mobile_dislocations[_qp] = _mobile_old + _mobile_dislocation_increment;
  _immobile_dislocations[_qp] = _immobile_old + _immobile_dislocation_increment;

  for (unsigned int i = 0; i < _num_inputs; i++)
    checkInputWindow(_input_values[i], _window_failure[i], _global_limits[i]);

  if (_verbose)
  {
    Moose::err << " Finalized ROM output\n";
    Moose::err << "  effective creep strain increment: "
               << std::sqrt(2.0 / 3.0 *
                            MetaPhysicL::raw_value(plastic_strain_increment.doubleContraction(
                                plastic_strain_increment)))
               << "\n";
    Moose::err << "  total effective creep strain: "
               << std::sqrt(2.0 / 3.0 *
                            MetaPhysicL::raw_value(
                                _creep_strain[_qp].doubleContraction(_creep_strain[_qp])))
               << "\n";
    Moose::err << "  creep rate: " << MetaPhysicL::raw_value(_creep_rate[_qp]) << "\n";
    Moose::err << "  mobile dislocation rate: " << MetaPhysicL::raw_value(_mobile_rate[_qp])
               << "\n";
    Moose::err << "  immobile dislocation rate: " << MetaPhysicL::raw_value(_immobile_rate[_qp])
               << "\n";
    Moose::err << "  new mobile dislocations: " << MetaPhysicL::raw_value(_mobile_dislocations[_qp])
               << "\n";
    Moose::err << "  new immobile dislocations: "
               << MetaPhysicL::raw_value(_immobile_dislocations[_qp]) << "\n"
               << std::endl;
  }

  ADRadialReturnCreepStressUpdateBase::computeStressFinalize(plastic_strain_increment);
}

Real
ADLAROMANCEStressUpdateBase::computeTimeStepLimit()
{
  Real limited_dt = ADRadialReturnStressUpdate::computeTimeStepLimit();

  Real mobile_strain_inc = std::abs(MetaPhysicL::raw_value(_mobile_dislocation_increment));
  if (mobile_strain_inc && _mobile_old)
    limited_dt =
        std::min(limited_dt, _dt * _max_mobile_increment * _mobile_old / mobile_strain_inc);
  Real immobile_strain_inc = std::abs(MetaPhysicL::raw_value(_immobile_dislocation_increment));
  if (immobile_strain_inc && _immobile_old)
    limited_dt =
        std::min(limited_dt, _dt * _max_immobile_increment * _immobile_old / immobile_strain_inc);

  return limited_dt;
}

ADReal
ADLAROMANCEStressUpdateBase::sigmoid(const Real lower,
                         const Real upper,
                         const ADReal & val,
                         const bool derivative)
{
  ADReal x = (val - lower) / (upper - lower);
  x = 2.0 * x - 1.0;
  ADReal plus = std::exp(-2.0 / (1.0 + x));
  ADReal minus = std::exp(-2.0 / (1.0 - x));
  if (!derivative)
    return 1.0 - plus / (plus + minus);

  ADReal dplus = plus * 2.0 / Utility::pow<2>(1.0 + x);
  ADReal dminus = -minus * 2.0 / Utility::pow<2>(1.0 - x);

  return (plus * dminus - dplus * minus) / Utility::pow<2>(plus + minus) * 2.0 / (upper - lower);
}
