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

defineADValidParams(
    ADLAROMANCEStressUpdateBase,
    ADRadialReturnCreepStressUpdateBase,
    params.addClassDescription(
        "Calculates the effective creep strain based on the rates predicted by a material "
        "specific Los Alamos Reduced Order Model derived from a Visco-Plastic Self Consistent "
        "calculations.");

    params.addRequiredCoupledVar("temperature", "The coupled temperature (K)");
    params.addCoupledVar("environmental_factor", 0.0, "Optional coupled environmental factor");
    params.addRangeCheckedParam<Real>("input_window_limit",
                                      1.0,
                                      "input_window_limit>0.0",
                                      "Multiplier for the input minium/maximum input window");
    MooseEnum window_failure("ERROR WARN IGNORE", "WARN");
    params.addParam<MooseEnum>("input_window_failure_action",
                               window_failure,
                               "What to do if ROM input is outside the window of applicability.");
    params.addParam<bool>("extrapolate_to_zero_stress",
                          true,
                          "Flag to allow for extrapolation of input J2 stress to zero");

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
    params.addRangeCheckedParam<Real>(
        "initial_immobile_dislocation_density",
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
        "Advanced"););

template <ComputeStage compute_stage>
ADLAROMANCEStressUpdateBase<compute_stage>::ADLAROMANCEStressUpdateBase(
    const InputParameters & parameters)
  : ADRadialReturnCreepStressUpdateBase<compute_stage>(parameters),
    _temperature(adCoupledValue("temperature")),
    _environmental(adCoupledValue("environmental_factor")),
    _window(getParam<Real>("input_window_limit")),
    _window_failure(
        parameters.get<MooseEnum>("input_window_failure_action").getEnum<WindowFailure>()),
    _extrapolate_stress(getParam<bool>("extrapolate_to_zero_stress")),
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
    _stress_index(2),

    _creep_strain_old_forcing_function(isParamValid("old_creep_strain_forcing_function")
                                           ? &getFunction("old_creep_strain_forcing_function")
                                           : NULL),

    _creep_rate(declareADProperty<Real>(_base_name + "creep_rate")),
    _failed(declareProperty<Real>("ROM_failure")),

    _derivative(0.0)
{
}

template <ComputeStage compute_stage>
void
ADLAROMANCEStressUpdateBase<compute_stage>::initialSetup()
{
  _transform = getTransform();

  _num_outputs = _transform.size();
  if (_num_outputs != 3)
    mooseError("In ", _name, ": _num_outputs (", _num_outputs, ") is not 3");

  _num_inputs = _transform[0].size();
  if (_num_inputs != 5 && _num_inputs != 6)
    mooseError("In ", _name, ": _num_inputs (", _num_inputs, ") is not 5 or 6");
  _use_env = _num_inputs == 6 ? true : false;

  _transform_coefs = getTransformCoefs();
  if (_transform_coefs.size() != _num_outputs || _transform_coefs[0].size() != _num_inputs)
    mooseError("In ", _name, ": transform_coef is the wrong shape!");

  _input_limits = getInputLimits();
  if (_input_limits.size() != _num_inputs || _input_limits[0].size() != 2)
    mooseError("In ", _name, ": input_limits is the wrong shape!");

  _coefs = getCoefs();
  if (_coefs.size() != _num_outputs)
    mooseError("In ", _name, ": coefs is the wrong shape!");

  _num_coefs = _coefs[0].size();
  _degree = std::pow(_num_coefs, 1.0 / _num_inputs);
  if (!_degree || _degree > 4)
    mooseError("In ", _name, ": degree must be 1, 2, 3 or 4.");

  _transformed_limits = getTransformedLimits();
  _makeframe_helper = getMakeFrameHelper();

  Moose::out << "ROM model info:\n  name:\t" << _name << "\n  number of outputs:\t" << _num_outputs
             << "\n  number of inputs:\t" << _num_inputs
             << "\n  degree (max Legendre degree + constant):\t" << _degree
             << "\n  number of coefficients:\t" << _num_coefs << std::endl;
}

template <ComputeStage compute_stage>
void
ADLAROMANCEStressUpdateBase<compute_stage>::initQpStatefulProperties()
{
  _mobile_dislocations[_qp] = _initial_mobile_dislocations;
  _immobile_dislocations[_qp] = _initial_immobile_dislocations;

  ADRadialReturnCreepStressUpdateBase<compute_stage>::initQpStatefulProperties();
}

template <ComputeStage compute_stage>
ADReal
ADLAROMANCEStressUpdateBase<compute_stage>::computeResidual(const ADReal & effective_trial_stress,
                                                            const ADReal & scalar)
{
  if (_immobile_function)
    _immobile_old = _immobile_function->value(_t, _q_point[_qp]);
  else
    _immobile_old = _immobile_dislocations_old[_qp];

  if (_mobile_function)
    _mobile_old = _mobile_function->value(_t, _q_point[_qp]);
  else
    _mobile_old = _mobile_dislocations_old[_qp];

  const ADReal trial_stress_mpa = (effective_trial_stress - _three_shear_modulus * scalar) * 1.0e-6;

  if (trial_stress_mpa < 0.0)
    mooseException("In ",
                   _name,
                   ": previously calculated scalar (",
                   MetaPhysicL::raw_value(scalar),
                   ") is too high resulting in a negative trial stress (",
                   MetaPhysicL::raw_value(trial_stress_mpa),
                   "). Cutting timestep.");

  Real effective_strain_old;
  if (_creep_strain_old_forcing_function)
    effective_strain_old = _creep_strain_old_forcing_function->value(_t, _q_point[_qp]);
  else
    effective_strain_old =
        std::sqrt(_creep_strain_old[_qp].doubleContraction(_creep_strain_old[_qp]) / 1.5);

  ADReal rom_effective_strain = 0.0;
  ADReal derivative_rom_effective_strain = 0.0;

  computeROMStrainRate(_dt,
                       _mobile_old,
                       _immobile_old,
                       trial_stress_mpa,
                       effective_strain_old,
                       _temperature[_qp],
                       _environmental[_qp],
                       _mobile_dislocation_increment,
                       _immobile_dislocation_increment,
                       rom_effective_strain,
                       derivative_rom_effective_strain);

  if (_verbose && compute_stage == RESIDUAL)
  {
    Moose::out << "Verbose information from " << _name << ": \n";
    Moose::out << "  dt: " << _dt << "\n";
    Moose::out << "  old mobile disl: " << _mobile_old << "\n";
    Moose::out << "  old immobile disl: " << _immobile_old << "\n";
    Moose::out << "  initial stress (MPa): " << effective_trial_stress * 1.0e-6 << "\n";
    Moose::out << "  temperature: " << _temperature[_qp] << "\n";
    Moose::out << "  environmental factor: " << _environmental[_qp] << "\n";
    Moose::out << "  calculated scalar strain value: " << scalar << "\n";
    Moose::out << "  trial stress into rom (MPa): " << trial_stress_mpa << "\n";
    Moose::out << "  old effective strain: " << effective_strain_old << "\n";
    Moose::out << " ROM outputs \n";
    Moose::out << "  effective incremental strain from rom: " << rom_effective_strain << "\n";
    Moose::out << "  new effective strain: " << effective_strain_old + rom_effective_strain << "\n";
    Moose::out << "  new mobile dislocations: " << _mobile_old + _mobile_dislocation_increment
               << "\n";
    Moose::out << "  new immobile dislocations: " << _immobile_old + _immobile_dislocation_increment
               << "\n"
               << std::endl;
  }

  _creep_rate[_qp] = rom_effective_strain / _dt;
  _derivative = derivative_rom_effective_strain * -_three_shear_modulus * 1.0e-6 - 1.0;

  return rom_effective_strain - scalar;
}

template <ComputeStage compute_stage>
void
ADLAROMANCEStressUpdateBase<compute_stage>::computeStressFinalize(
    const ADRankTwoTensor & plastic_strain_increment)
{
  _mobile_dislocations[_qp] = _mobile_old + _mobile_dislocation_increment;
  _immobile_dislocations[_qp] = _immobile_old + _immobile_dislocation_increment;

  if (_verbose && compute_stage == RESIDUAL)
  {
    Moose::out << "Finalized verbose information from " << _name << "\n";
    Moose::out << "  increment effective creep strain: "
               << std::sqrt(2.0 / 3.0 *
                            plastic_strain_increment.doubleContraction(plastic_strain_increment))
               << "\n";
    Moose::out << "  effective_creep_strain: "
               << std::sqrt(2.0 / 3.0 * _creep_strain[_qp].doubleContraction(_creep_strain[_qp]))
               << "\n";
    Moose::out << "  new mobile dislocations: " << _mobile_dislocations[_qp] << "\n";
    Moose::out << "  new immobile dislocations: " << _immobile_dislocations[_qp] << "\n"
               << std::endl;
  }

  ADRadialReturnCreepStressUpdateBase<compute_stage>::computeStressFinalize(
      plastic_strain_increment);
}

template <ComputeStage compute_stage>
Real
ADLAROMANCEStressUpdateBase<compute_stage>::computeTimeStepLimit()
{
  Real limited_dt = ADRadialReturnStressUpdate<compute_stage>::computeTimeStepLimit();

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

template <ComputeStage compute_stage>
void
ADLAROMANCEStressUpdateBase<compute_stage>::computeROMStrainRate(
    const Real dt,
    const Real & mobile_dislocations_old,
    const Real & immobile_dislocations_old,
    const ADReal & trial_stress,
    const Real & effective_strain_old,
    const ADReal & temperature,
    const ADReal & environmental,
    ADReal & mobile_dislocation_increment,
    ADReal & immobile_dislocation_increment,
    ADReal & rom_effective_strain,
    ADReal & rom_effective_strain_derivative)
{
  // Prepare input
  std::vector<ADReal> input_values = {mobile_dislocations_old,
                                      immobile_dislocations_old,
                                      trial_stress,
                                      effective_strain_old,
                                      temperature};
  if (_use_env)
    input_values.push_back(environmental);

  std::vector<std::vector<ADReal>> rom_inputs(_num_outputs, std::vector<ADReal>(_num_inputs));
  std::vector<std::vector<ADReal>> drom_inputs(_num_outputs, std::vector<ADReal>(_num_inputs));

  checkInputWindows(input_values);
  convertInput(input_values, rom_inputs, drom_inputs);

  std::vector<ADReal> old_input_values = {
      mobile_dislocations_old, immobile_dislocations_old, effective_strain_old};

  std::vector<std::vector<std::vector<ADReal>>> polynomial_inputs(
      _num_outputs, std::vector<std::vector<ADReal>>(_num_inputs, std::vector<ADReal>(_degree)));
  std::vector<ADReal> rom_outputs(_num_outputs);
  std::vector<ADReal> input_value_increments(_num_outputs);

  std::vector<std::vector<std::vector<ADReal>>> dpolynomial_inputs(
      _num_outputs, std::vector<std::vector<ADReal>>(_num_inputs, std::vector<ADReal>(_degree)));
  std::vector<ADReal> drom_outputs(_num_outputs);
  std::vector<ADReal> dinput_value_increments(_num_outputs);

  buildPolynomials(rom_inputs, drom_inputs, polynomial_inputs, dpolynomial_inputs);
  computeValues(_coefs, polynomial_inputs, dpolynomial_inputs, rom_outputs, drom_outputs);
  convertOutput(dt,
                old_input_values,
                rom_outputs,
                drom_outputs,
                input_value_increments,
                dinput_value_increments);

  mobile_dislocation_increment = input_value_increments[0];
  immobile_dislocation_increment = input_value_increments[1];
  rom_effective_strain = input_value_increments[2];

  rom_effective_strain_derivative = dinput_value_increments[2];
}

template <ComputeStage compute_stage>
void
ADLAROMANCEStressUpdateBase<compute_stage>::checkInputWindows(std::vector<ADReal> & input)
{
  if (compute_stage != RESIDUAL)
    return;

  _failed[_qp] = 0.0;
  for (unsigned int i = 0; i < _num_outputs; ++i)
  {
    for (unsigned int j = 0; j < _num_inputs; ++j)
    {
      Real high_limit = _input_limits[j][1] * _window;
      Real low_limit = _input_limits[j][0] * (2.0 - _window);
      if (j == _stress_index && _extrapolate_stress)
        low_limit = 0.0;
      if (input[j] < low_limit || input[j] > high_limit)
      {
        _failed[_qp] += (j + 1) * (j + 1);
        if (_window_failure == WindowFailure::WARN)
          mooseWarning("In ",
                       _name,
                       ": input parameter number input=",
                       j,
                       " output=",
                       i,
                       " with value (",
                       input[j],
                       ") is out of range (",
                       _input_limits[j][0],
                       " - ",
                       _input_limits[j][1],
                       "), window (",
                       _window,
                       ")");
        else if (_window_failure == WindowFailure::ERROR)
          mooseError("In ",
                     _name,
                     ": input parameter number input=",
                     j,
                     " output=",
                     i,
                     " with value (",
                     input[j],
                     ") is out of range (",
                     _input_limits[j][0],
                     " - ",
                     _input_limits[j][1],
                     "), window (",
                     _window,
                     ")");
      }
    }
  }
}

template <ComputeStage compute_stage>
void
ADLAROMANCEStressUpdateBase<compute_stage>::convertInput(
    const std::vector<ADReal> & input,
    std::vector<std::vector<ADReal>> & converted,
    std::vector<std::vector<ADReal>> & dconverted)
{
  for (unsigned int i = 0; i < _num_outputs; ++i)
  {
    for (unsigned int j = 0; j < _num_inputs; ++j)
    {
      ADReal x = input[j];
      ADReal dx = 1.0;
      if (_transform[i][j] == ROMInputTransform::EXP)
      {
        x = std::exp(x / _transform_coefs[i][j]);
        dx = x / _transform_coefs[i][j];
      }
      else if (_transform[i][j] == ROMInputTransform::LOG)
      {
        dx = 1.0 / (x + _transform_coefs[i][j]);
        x = std::log(x + _transform_coefs[i][j]);
      }

      converted[i][j] = 2.0 * (x - _transformed_limits[i][j][0]) /
                            (_transformed_limits[i][j][1] - _transformed_limits[i][j][0]) -
                        1.0;
      if (j == _stress_index)
        dconverted[i][j] = dx * 2.0 / (_transformed_limits[i][j][1] - _transformed_limits[i][j][0]);
      else
        dconverted[i][j] = 0.0;
    }
  }
}

template <ComputeStage compute_stage>
void
ADLAROMANCEStressUpdateBase<compute_stage>::buildPolynomials(
    const std::vector<std::vector<ADReal>> & rom_inputs,
    const std::vector<std::vector<ADReal>> & drom_inputs,
    std::vector<std::vector<std::vector<ADReal>>> & polynomial_inputs,
    std::vector<std::vector<std::vector<ADReal>>> & dpolynomial_inputs)
{
  for (unsigned int i = 0; i < _num_outputs; ++i)
  {
    for (unsigned int j = 0; j < _num_inputs; ++j)
    {
      for (unsigned int k = 0; k < _degree; ++k)
      {
        polynomial_inputs[i][j][k] = computePolynomial(rom_inputs[i][j], k);

        if (j != _stress_index)
          dpolynomial_inputs[i][j][k] = 0.0;
        else
        {
          // This is not the true derivative, but rather an optimization for computeValues
          dpolynomial_inputs[i][j][k] = drom_inputs[i][j] *
                                        computePolynomial(rom_inputs[i][j], k, true) /
                                        polynomial_inputs[i][j][k];
        }
      }
    }
  }
}

template <ComputeStage compute_stage>
void
ADLAROMANCEStressUpdateBase<compute_stage>::computeValues(
    const std::vector<std::vector<Real>> & coefs,
    const std::vector<std::vector<std::vector<ADReal>>> & polynomial_inputs,
    const std::vector<std::vector<std::vector<ADReal>>> & dpolynomial_inputs,
    std::vector<ADReal> & rom_outputs,
    std::vector<ADReal> & drom_outputs)
{
  for (unsigned int i = 0; i < _num_outputs; ++i)
  {
    rom_outputs[i] = 0.0;
    drom_outputs[i] = 0.0;
    for (unsigned int j = 0; j < _num_coefs; ++j)
    {
      ADReal xvals = coefs[i][j];
      ADReal dxvals = 0.0;
      for (unsigned int k = 0; k < _num_inputs; ++k)
      {
        xvals *= polynomial_inputs[i][k][_makeframe_helper[j][k]];
        dxvals += dpolynomial_inputs[i][k][_makeframe_helper[j][k]];
      }

      rom_outputs[i] += xvals;
      drom_outputs[i] += dxvals * xvals;
    }
  }
}

template <ComputeStage compute_stage>
void
ADLAROMANCEStressUpdateBase<compute_stage>::convertOutput(
    const Real dt,
    const std::vector<ADReal> & old_input_values,
    const std::vector<ADReal> & rom_outputs,
    const std::vector<ADReal> & drom_outputs,
    std::vector<ADReal> & input_value_increments,
    std::vector<ADReal> & dinput_value_increments)
{
  for (unsigned int i = 0; i < _num_outputs; ++i)
  {
    input_value_increments[i] = std::exp(rom_outputs[i]) * dt;
    if (i == 0 || i == 1)
    {
      dinput_value_increments[i] = 0.0;
      input_value_increments[i] *= old_input_values[i];
      if (i == 0)
        input_value_increments[i] *= -1.0;
    }
    else
      dinput_value_increments[i] = input_value_increments[i] * drom_outputs[i];
  }
}

template <ComputeStage compute_stage>
ADReal
ADLAROMANCEStressUpdateBase<compute_stage>::computePolynomial(const ADReal & value,
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

template <ComputeStage compute_stage>
std::vector<std::vector<std::vector<Real>>>
ADLAROMANCEStressUpdateBase<compute_stage>::getTransformedLimits() const
{
  std::vector<std::vector<std::vector<Real>>> transformed_limits(
      _num_outputs, std::vector<std::vector<Real>>(_num_inputs, std::vector<Real>(2)));

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
    }
  }

  return transformed_limits;
}

template <ComputeStage compute_stage>
std::vector<std::vector<unsigned int>>
ADLAROMANCEStressUpdateBase<compute_stage>::getMakeFrameHelper() const
{
  std::vector<std::vector<unsigned int>> v(_num_coefs, std::vector<unsigned int>(_num_inputs));

  for (unsigned int numcoeffs = 0; numcoeffs < _num_coefs; ++numcoeffs)
    for (unsigned int invar = 0; invar < _num_inputs; ++invar)
      v[numcoeffs][invar] = numcoeffs / MathUtils::pow(_degree, invar) % _degree;

  return v;
}

// explicit instantiation is required for AD base classes
adBaseClass(ADLAROMANCEStressUpdateBase);
