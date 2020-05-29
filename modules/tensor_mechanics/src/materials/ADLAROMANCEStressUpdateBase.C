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
  MooseEnum window_failure("ERROR WARN IGNORE EXTRAPOLATE", "EXTRAPOLATE");
  MooseEnum dislocation_window_failure("ERROR WARN IGNORE EXTRAPOLATE", "ERROR");
  params.addParam<MooseEnum>(
      "mobile_input_window_failure",
      dislocation_window_failure,
      "What to do if mobile dislocation concentration is outside the window of applicability.");
  params.addParam<MooseEnum>(
      "immobile_input_window_failure",
      dislocation_window_failure,
      "What to do if immobile dislocation concentration is outside the window of applicability.");
  params.addParam<MooseEnum>("stress_input_window_failure",
                             window_failure,
                             "What to do if stress is outside the window of applicability.");
  params.addParam<MooseEnum>("old_strain_input_window_failure",
                             window_failure,
                             "What to do if old strain is outside the window of applicability.");
  params.addParam<MooseEnum>("temperature_input_window_failure",
                             window_failure,
                             "What to do if temperature is outside the window of applicability.");
  params.addParam<MooseEnum>(
      "environment_input_window_failure",
      window_failure,
      "What to do if environmental factor is outside the window of applicability.");

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
    _extrapolation(declareADProperty<Real>("ROM_extrapolation")),

    _derivative(0.0),
    _non_stress_extrapolation(1.0),
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

  _num_outputs = _transform.size();
  if (_num_outputs != 3)
    mooseError("In ",
               _name,
               ": (",
               _num_outputs,
               ") detected. Three and only three outputs are currently supported.");

  _num_inputs = _transform[0].size();
  if (_num_inputs != 5 && _num_inputs != 6)
    mooseError("In ", _name, ": _num_inputs (", _num_inputs, ") is not 5 or 6");
  if (_num_inputs == 6 && !_environmental)
    paramError("environmental",
               "Number of ROM inputs indicate environmental factor is required to be coupled.");
  if (_num_inputs != 6 && _environmental)
    paramError("environmental",
               "Number of ROM inputs indicate environmental factor is not implemented, but "
               "environmental factor coupled.");

  _input_values.resize(_num_inputs);

  _transform_coefs = getTransformCoefs();
  if (_transform_coefs.size() != _num_outputs || _transform_coefs[0].size() != _num_inputs)
    mooseError("In ", _name, ": transform_coef is the wrong shape!");

  /// Check for divide by zero
  for (unsigned int o = 0; o < _num_outputs; ++o)
    for (unsigned int i = 0; i < _num_inputs; ++i)
      if (_transform[o][i] == ROMInputTransform::EXP && _transform_coefs[o][i] == 0)
        mooseError("In ",
                   _name,
                   ": One of the transform coefficients is 0 with an exponential transform. This "
                   "will lead to a divide-by-zero error. Check the ROM parameters");

  _input_limits = getInputLimits();
  if (_input_limits.size() != _num_inputs || _input_limits[0].size() != 2)
    mooseError("In ", _name, ": input_limits is the wrong shape!");

  _coefs = getCoefs();
  if (_coefs.size() != _num_outputs)
    mooseError("In ", _name, ": coefs is the wrong shape!");

  _num_coefs = _coefs[0].size();
  _precomputed_vals.resize(_num_coefs);

  _degree = std::pow(_num_coefs, 1.0 / _num_inputs);
  if (!_degree || _degree > 4)
    mooseError("In ", _name, ": degree must be 1, 2, 3 or 4.");

  _transformed_limits = getTransformedLimits();
  _makeframe_helper = getMakeFrameHelper();

  _rom_inputs.resize(_num_inputs);
  _polynomial_inputs.resize(_num_inputs);
  for (unsigned i = 0; i < _num_inputs; i++)
    _polynomial_inputs[i].resize(_degree);

  if (_verbose)
    Moose::err << "ROM model info:\n  name:\t" << _name << "\n  number of outputs:\t"
               << _num_outputs << "\n  number of inputs:\t" << _num_inputs
               << "\n  degree (max Legendre degree + constant):\t" << _degree
               << "\n  number of coefficients:\t" << _num_coefs << std::endl;
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
  // Make maximum allowed scalar a little bit less than the deformation that would reduce the trial
  // stress to zero. This prevents negative trial stresses.
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

  // Check to see if input is in range
  _non_stress_extrapolation = 1.0;
  for (unsigned int j = 0; j < _num_inputs; j++)
    if (j != _stress_input_idx)
      _non_stress_extrapolation *=
          checkInputWindow(_input_values[j], _input_limits[j], _window_failure[j]);

  // Precompute transformed input and prebuild polynomials for inputs other than strain
  if (_non_stress_extrapolation)
    precomputeROM(_strain_output_idx);
}

void
ADLAROMANCEStressUpdateBase::precomputeROM(const unsigned out_idx)
{
  for (unsigned int i = 0; i < _num_inputs; ++i)
  {
    if (i != _stress_input_idx)
    {
      _rom_inputs[i] = convertInput(_input_values[i],
                                    _transform[out_idx][i],
                                    _transform_coefs[out_idx][i],
                                    _transformed_limits[out_idx][i]);
      buildPolynomials(_rom_inputs[i], _polynomial_inputs[i]);
    }
  }
  precomputeValues(_coefs[out_idx], _polynomial_inputs, _precomputed_vals);
}

ADReal
ADLAROMANCEStressUpdateBase::computeResidual(const ADReal & effective_trial_stress,
                                             const ADReal & scalar)
{
  ADReal trial_stress_mpa = effective_trial_stress * 1.0e-6;
  ADReal dtrial_stress_dscalar = 0.0;
  if (_apply_strain)
  {
    trial_stress_mpa -= _three_shear_modulus * scalar * 1.0e-6;
    dtrial_stress_dscalar -= _three_shear_modulus * 1.0e-6;
  }
  _input_values[_stress_input_idx] = trial_stress_mpa;

  ADReal rom_effective_strain_inc = 0.0;
  ADReal drom_effective_strain_inc_dstress = 0.0;

  // Check window of new trial stress
  _extrapolation[_qp] =
      _non_stress_extrapolation * checkInputWindow(_input_values[_stress_input_idx],
                                                   _input_limits[_stress_input_idx],
                                                   _window_failure[_stress_input_idx]);

  // Run ROM if all values are within windows.
  if (_extrapolation[_qp])
  {
    rom_effective_strain_inc = computeROM(_strain_output_idx);
    drom_effective_strain_inc_dstress = computeROM(_strain_output_idx, true);

    // apply extrapolation
    rom_effective_strain_inc *= _extrapolation[_qp];
    const ADReal dextrapolation_dstress =
        _non_stress_extrapolation * checkInputWindow(_input_values[_stress_input_idx],
                                                     _input_limits[_stress_input_idx],
                                                     _window_failure[_stress_input_idx],
                                                     true);
    drom_effective_strain_inc_dstress = drom_effective_strain_inc_dstress * _extrapolation[_qp] +
                                        rom_effective_strain_inc * dextrapolation_dstress;
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
    Moose::err << "  non-stress extrapolation: "
               << MetaPhysicL::raw_value(_non_stress_extrapolation) << "\n";
    Moose::err << "  extrapolation: " << MetaPhysicL::raw_value(_extrapolation[_qp]) << "\n";
    Moose::err << "  effective strain increment: "
               << MetaPhysicL::raw_value(rom_effective_strain_inc) << std::endl;
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

  _creep_rate[_qp] = rom_effective_strain_inc / _dt;
  _derivative = drom_effective_strain_inc_dstress * dtrial_stress_dscalar - 1.0;

  if (!_apply_strain)
  {
    if (_verbose)
      Moose::err << "    Strain not applied due to apply_strain input parameter!" << std::endl;
    return 0.0;
  }
  return rom_effective_strain_inc - scalar;
}

ADReal
ADLAROMANCEStressUpdateBase::computeROM(const unsigned out_idx, const bool derivative)
{
  // Update due to new stress
  _rom_inputs[_stress_input_idx] = convertInput(_input_values[_stress_input_idx],
                                                _transform[out_idx][_stress_input_idx],
                                                _transform_coefs[out_idx][_stress_input_idx],
                                                _transformed_limits[out_idx][_stress_input_idx]);
  buildPolynomials(_rom_inputs[_stress_input_idx], _polynomial_inputs[_stress_input_idx]);

  // Compute ROM values
  const ADReal rom_outputs = computeValues(_precomputed_vals, _polynomial_inputs);

  // Return converted output if not derivative
  if (!derivative)
    return convertOutput(_old_input_values, rom_outputs, out_idx);

  const ADReal drom_input = convertInput(_input_values[_stress_input_idx],
                                         _transform[out_idx][_stress_input_idx],
                                         _transform_coefs[out_idx][_stress_input_idx],
                                         _transformed_limits[out_idx][_stress_input_idx],
                                         derivative);

  std::vector<ADReal> dpolynomial_inputs(_degree, 0.0);
  buildPolynomials(_rom_inputs[_stress_input_idx], dpolynomial_inputs, drom_input, derivative);

  const ADReal drom_output =
      computeValues(_precomputed_vals, _polynomial_inputs, dpolynomial_inputs, derivative);

  return convertOutput(_old_input_values, rom_outputs, out_idx, drom_output, derivative);
}

ADReal
ADLAROMANCEStressUpdateBase::checkInputWindow(ADReal & input,
                                              const std::vector<Real> & limits,
                                              const WindowFailure behavior,
                                              const bool derivative)
{
  ADReal smoother_step = 1.0;
  if (derivative)
    smoother_step = 0.0;
  if (behavior == WindowFailure::IGNORE)
    return smoother_step;

  if (input < limits[0] || input > limits[1])
  {
    if (behavior == WindowFailure::EXTRAPOLATE)
    {
      if (input < limits[0])
      {
        smoother_step = MathUtils::smootherStep(input, 0.0, limits[0], derivative);
        input = limits[0];
      }
      else
        input = limits[1];
    }
    else
    {
      std::stringstream msg;
      msg << "In " << _name << ": input parameter with value (" << MetaPhysicL::raw_value(input)
          << ") is out of range (" << limits[0] << " - " << limits[1] << ")";

      if (behavior == WindowFailure::WARN)
        mooseWarning(msg.str());
      else if (behavior == WindowFailure::ERROR)
        mooseError(msg.str());
    }
  }

  return smoother_step;
}

ADReal
ADLAROMANCEStressUpdateBase::convertInput(const ADReal & input,
                                          const ROMInputTransform transform,
                                          const Real transform_coef,
                                          const std::vector<Real> & transformed_limits,
                                          const bool derivative)
{
  ADReal x(1.0);
  if (transform == ROMInputTransform::LINEAR)
  {
    if (!derivative)
      x = input;
  }
  else if (transform == ROMInputTransform::EXP)
  {
    if (!derivative)
      x = std::exp(input / transform_coef);
    else
      x = std::exp(input / transform_coef) / transform_coef;
  }
  else // ROMInputTransform::LOG
  {
    if (!derivative)
      x = std::log(input + transform_coef);
    else
      x = 1.0 / (input + transform_coef);
  }

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
ADLAROMANCEStressUpdateBase::precomputeValues(
    const std::vector<Real> & coefs,
    const std::vector<std::vector<ADReal>> & polynomial_inputs,
    std::vector<ADReal> & precomputed)
{
  for (unsigned int j = 0; j < _num_coefs; ++j)
  {
    precomputed[j] = coefs[j];
    for (unsigned int i = 0; i < _num_inputs; ++i)
      if (i != _stress_input_idx)
        precomputed[j] *= polynomial_inputs[i][_makeframe_helper[j + _num_coefs * i]];
  }
}

ADReal
ADLAROMANCEStressUpdateBase::computeValues(
    const std::vector<ADReal> & precomputed,
    const std::vector<std::vector<ADReal>> & polynomial_inputs,
    const std::vector<ADReal> & dpolynomial_inputs,
    const bool derivative)
{
  ADReal rom_output = 0.0;
  for (unsigned int j = 0; j < _num_coefs; ++j)
  {
    ADReal vals =
        precomputed[j] *
        polynomial_inputs[_stress_input_idx][_makeframe_helper[j + _num_coefs * _stress_input_idx]];

    if (!derivative)
      rom_output += vals;
    else
      rom_output +=
          dpolynomial_inputs[_makeframe_helper[j + _num_coefs * _stress_input_idx]] * vals;
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
  if (derivative && out_idx != _strain_output_idx)
    return 0.0;

  ADReal ROM_computed_increments = std::exp(rom_output) * _dt;
  if (derivative)
    ROM_computed_increments *= drom_output;
  else if (out_idx != _strain_output_idx)
    ROM_computed_increments *= -old_input_values[out_idx];
  return ROM_computed_increments;
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

std::vector<std::vector<std::vector<Real>>>
ADLAROMANCEStressUpdateBase::getTransformedLimits() const
{
  std::vector<std::vector<std::vector<Real>>> transformed_limits(
      _num_outputs, std::vector<std::vector<Real>>(_num_inputs, std::vector<Real>(3)));

  for (unsigned int i = 0; i < _num_outputs; ++i)
  {
    for (unsigned int j = 0; j < _num_inputs; ++j)
    {
      for (unsigned int k = 0; k < 2; ++k)
      {
        if (_transform[i][j] == ROMInputTransform::EXP)
          transformed_limits[i][j][k] = std::exp(_input_limits[j][k] / _transform_coefs[i][j]);
        else if (_transform[i][j] == ROMInputTransform::LOG)
          transformed_limits[i][j][k] = std::log(_input_limits[j][k] + _transform_coefs[i][j]);
        else
          transformed_limits[i][j][k] = _input_limits[j][k];
      }
      // Pre-compute the difference between the two limits divided by two for later input
      // conversion
      transformed_limits[i][j][2] =
          (transformed_limits[i][j][1] - transformed_limits[i][j][0]) / 2.0;
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

  if (_extrapolation[_qp])
  {
    precomputeROM(_mobile_output_idx);
    _mobile_dislocation_increment = computeROM(_mobile_output_idx) * _extrapolation[_qp];

    precomputeROM(_immobile_output_idx);
    _immobile_dislocation_increment = computeROM(_immobile_output_idx) * _extrapolation[_qp];
  }

  _mobile_dislocations[_qp] = _mobile_old + _mobile_dislocation_increment;
  _immobile_dislocations[_qp] = _immobile_old + _immobile_dislocation_increment;

  for (unsigned int j = 0; j < _num_inputs; j++)
    if (_window_failure[j] == WindowFailure::WARN || _window_failure[j] == WindowFailure::ERROR)
      checkInputWindow(_input_values[j], _input_limits[j], _window_failure[j]);

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
