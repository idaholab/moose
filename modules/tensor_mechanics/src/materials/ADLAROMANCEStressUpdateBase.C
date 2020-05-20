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
  params.addRangeCheckedParam<Real>("input_window_limit",
                                    1.0,
                                    "input_window_limit>0.0",
                                    "Multiplier for the input minium/maximum input window");
  MooseEnum window_failure("ERROR WARN IGNORE EXTRAPOLATE", "WARN");
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
    _effective_strain_old(0.0),
    _stress_input_idx(2),
    _mobile_output_idx(0),
    _immobile_output_idx(1),
    _strain_output_idx(2),

    _creep_strain_old_forcing_function(isParamValid("old_creep_strain_forcing_function")
                                           ? &getFunction("old_creep_strain_forcing_function")
                                           : NULL),

    _creep_rate(declareADProperty<Real>(_base_name + "creep_rate")),
    _extrapolation(declareADProperty<Real>("ROM_extrapolation")),

    _derivative(0.0),
    _input_within_range(true),
    _run_ROM(true),
    _old_input_values(3)
{
  _check_range = true;
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
  return (effective_trial_stress - 1.0e-9) / _three_shear_modulus;
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
  _input_values[0] = _mobile_old;
  _input_values[1] = _immobile_old;
  _input_values[_stress_input_idx] = effective_trial_stress * 1.0e-6;
  _input_values[3] = _effective_strain_old;
  _input_values[4] = _temperature[_qp];
  if (_environmental)
    _input_values[5] = (*_environmental)[_qp];

  _old_input_values[_mobile_output_idx] = _mobile_old;
  _old_input_values[_immobile_output_idx] = _immobile_old;
  _old_input_values[_strain_output_idx] = _effective_strain_old;

  _input_within_range = true;
  for (unsigned int j = 0; j < _num_inputs; j++)
    if (j != _stress_input_idx)
      _input_within_range *= checkSpecificInputWindow(_input_values, j);
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
  _run_ROM = _input_within_range * checkSpecificInputWindow(_input_values, _stress_input_idx);

  // Run ROM if all values are within windows.
  if (_run_ROM)
  {
    rom_effective_strain_inc = computeROMStrainRate(_strain_output_idx);
    drom_effective_strain_inc_dstress = computeROMStrainRate(_strain_output_idx, true);
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
  }

  // Print out relevant verbose information, and quit creep stress calculation
  if (!_run_ROM)
  {
    if (scalar)
      mooseError("In ", _name, ": Internal error: Scalar (", scalar, ") isn't zero!");
    if (_verbose)
      Moose::err << "    ROM evalulation skipped due to out-of-range input!" << std::endl;

    _creep_rate[_qp] = 0.0;
    _derivative = 1.0;
    return 0.0;
  }

  // Extrapolate to 0 stress if applicable
  _extrapolation[_qp] = 1.0;
  if (_extrapolate_stress)
  {
    // Update stress given converged strain increment
    trial_stress_mpa =
        (effective_trial_stress - _three_shear_modulus * rom_effective_strain_inc) * 1.0e-6;

    // If stress is below the input limit, perform extrapolation with zero strain
    if (trial_stress_mpa < _input_limits[_stress_input_idx][0])
    {
      _extrapolation[_qp] =
          MathUtils::smootherStep(trial_stress_mpa, 0.0, _input_limits[_stress_input_idx][0]);
      const ADReal dextrapolation_dstress =
          MathUtils::smootherStep(trial_stress_mpa, 0.0, _input_limits[_stress_input_idx][0], true);

      rom_effective_strain_inc *= _extrapolation[_qp];
      drom_effective_strain_inc_dstress = drom_effective_strain_inc_dstress * _extrapolation[_qp] +
                                          rom_effective_strain_inc * dextrapolation_dstress;

      if (_verbose)
      {
        Moose::err << "  Stress extrapolated.\n    Smootherstep: "
                   << MetaPhysicL::raw_value(_extrapolation[_qp]) << "\n";
        Moose::err << "    new effective strain increment: "
                   << MetaPhysicL::raw_value(rom_effective_strain_inc) << std::endl;
      }
    }
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
ADLAROMANCEStressUpdateBase::computeROMStrainRate(const unsigned index, const bool jacobian)
{
  std::vector<ADReal> rom_inputs(_num_inputs, 0.0);
  std::vector<ADReal> drom_inputs(_num_inputs, 0.0);
  convertInput(_input_values, rom_inputs, drom_inputs, index);

  std::vector<std::vector<ADReal>> polynomial_inputs(_num_inputs,
                                                     std::vector<ADReal>(_degree, 0.0));
  std::vector<std::vector<ADReal>> dpolynomial_inputs(_num_inputs,
                                                      std::vector<ADReal>(_degree, 0.0));
  buildPolynomials(rom_inputs, drom_inputs, polynomial_inputs, dpolynomial_inputs);

  ADReal rom_outputs(0.0);
  ADReal drom_outputs(0.0);
  computeValues(_coefs[index], polynomial_inputs, dpolynomial_inputs, rom_outputs, drom_outputs);

  ADReal dROM_computed_increments(0.0);
  ADReal ROM_computed_increments(0.0);
  convertOutput(_old_input_values,
                rom_outputs,
                drom_outputs,
                ROM_computed_increments,
                dROM_computed_increments,
                index);

  if (jacobian)
    return dROM_computed_increments;
  return ROM_computed_increments;
}

bool
ADLAROMANCEStressUpdateBase::checkSpecificInputWindow(std::vector<ADReal> & input,
                                                      const unsigned int index)
{
  const Real high_limit = _input_limits[index][1] * _window;
  const Real low_limit = _input_limits[index][0] * (2.0 - _window);
  if (input[index] < low_limit || input[index] > high_limit)
  {
    if (input[index] < low_limit && index == _stress_input_idx && _extrapolate_stress)
      input[index] = low_limit;
    else
    {
      if (_window_failure == WindowFailure::WARN)
        mooseWarning("In ",
                     _name,
                     ": input parameter number input=",
                     index,
                     " with value (",
                     MetaPhysicL::raw_value(input[index]),
                     ") is out of range (",
                     _input_limits[index][0],
                     " - ",
                     _input_limits[index][1],
                     "), window (",
                     _window,
                     ")");
      else if (_window_failure == WindowFailure::ERROR)
        mooseError("In ",
                   _name,
                   ": input parameter number input=",
                   index,
                   " with value (",
                   MetaPhysicL::raw_value(input[index]),
                   ") is out of range (",
                   _input_limits[index][0],
                   " - ",
                   _input_limits[index][1],
                   "), window (",
                   _window,
                   ")");
      else if (_window_failure == WindowFailure::EXTRAPOLATE)
      {
        if (input[index] < low_limit)
          return false;
        if (input[index] > high_limit)
          input[index] = high_limit;
      }
    }
  }

  return true;
}

void
ADLAROMANCEStressUpdateBase::convertInput(const std::vector<ADReal> & input,
                                          std::vector<ADReal> & converted,
                                          std::vector<ADReal> & dconverted,
                                          const unsigned index)
{
  for (unsigned int j = 0; j < _num_inputs; ++j)
  {
    ADReal x = input[j];
    ADReal dx = 1.0;
    if (_transform[index][j] == ROMInputTransform::EXP)
    {
      x = std::exp(x / _transform_coefs[index][j]);
      dx = x / _transform_coefs[index][j];
    }
    else if (_transform[index][j] == ROMInputTransform::LOG)
    {
      dx = 1.0 / (x + _transform_coefs[index][j]);
      x = std::log(x + _transform_coefs[index][j]);
    }

    converted[j] = 2.0 * (x - _transformed_limits[index][j][0]) /
                       (_transformed_limits[index][j][1] - _transformed_limits[index][j][0]) -
                   1.0;
    if (j == _stress_input_idx)
      dconverted[j] =
          dx * 2.0 / (_transformed_limits[index][j][1] - _transformed_limits[index][j][0]);
  }
}

void
ADLAROMANCEStressUpdateBase::buildPolynomials(const std::vector<ADReal> & rom_inputs,
                                              const std::vector<ADReal> & drom_inputs,
                                              std::vector<std::vector<ADReal>> & polynomial_inputs,
                                              std::vector<std::vector<ADReal>> & dpolynomial_inputs)
{
  for (unsigned int j = 0; j < _num_inputs; ++j)
  {
    for (unsigned int k = 0; k < _degree; ++k)
    {
      polynomial_inputs[j][k] = computePolynomial(rom_inputs[j], k);
      if (j == _stress_input_idx)
        dpolynomial_inputs[j][k] = drom_inputs[j] * computePolynomial(rom_inputs[j], k, true);
    }
  }
}

void
ADLAROMANCEStressUpdateBase::computeValues(
    const std::vector<Real> & coefs,
    const std::vector<std::vector<ADReal>> & polynomial_inputs,
    const std::vector<std::vector<ADReal>> & dpolynomial_inputs,
    ADReal & rom_outputs,
    ADReal & drom_outputs)
{
  for (unsigned int j = 0; j < _num_coefs; ++j)
  {
    ADReal xvals = coefs[j];
    ADReal dxvals = 0.0;
    for (unsigned int k = 0; k < _num_inputs; ++k)
    {
      xvals *= polynomial_inputs[k][_makeframe_helper[j][k]];
      dxvals += dpolynomial_inputs[k][_makeframe_helper[j][k]] /
                polynomial_inputs[k][_makeframe_helper[j][k]];
    }

    rom_outputs += xvals;
    drom_outputs += dxvals * xvals;
  }
}

void
ADLAROMANCEStressUpdateBase::convertOutput(const std::vector<Real> & old_input_values,
                                           const ADReal & rom_outputs,
                                           const ADReal & drom_outputs,
                                           ADReal & ROM_computed_increments,
                                           ADReal & dROM_computed_increments,
                                           const unsigned index)
{

  ROM_computed_increments = std::exp(rom_outputs) * _dt;
  if (index == 2)
    dROM_computed_increments = ROM_computed_increments * drom_outputs;
  else
    ROM_computed_increments *= -old_input_values[index];
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

std::vector<std::vector<unsigned int>>
ADLAROMANCEStressUpdateBase::getMakeFrameHelper() const
{
  std::vector<std::vector<unsigned int>> v(_num_coefs, std::vector<unsigned int>(_num_inputs));

  for (unsigned int numcoeffs = 0; numcoeffs < _num_coefs; ++numcoeffs)
    for (unsigned int invar = 0; invar < _num_inputs; ++invar)
      v[numcoeffs][invar] = numcoeffs / MathUtils::pow(_degree, invar) % _degree;

  return v;
}

void
ADLAROMANCEStressUpdateBase::computeStressFinalize(const ADRankTwoTensor & plastic_strain_increment)
{
  _mobile_dislocation_increment = computeROMStrainRate(_immobile_output_idx) * _extrapolation[_qp];
  _immobile_dislocation_increment =
      computeROMStrainRate(_immobile_output_idx) * _extrapolation[_qp];
  _mobile_dislocations[_qp] = _mobile_old + _mobile_dislocation_increment;
  _immobile_dislocations[_qp] = _immobile_old + _immobile_dislocation_increment;

  if (_t_step > 0 &&
      (_current_execute_flag == EXEC_LINEAR || _current_execute_flag == EXEC_NONLINEAR))
  {
    if (_mobile_dislocations[_qp] < _input_limits[0][0] ||
        _mobile_dislocations[_qp] > _input_limits[0][1])
      mooseError("In ",
                 _name,
                 ": mobile dislocations (",
                 MetaPhysicL::raw_value(_mobile_dislocations[_qp]),
                 ") is out side of its limits (",
                 _input_limits[0][0],
                 " - ",
                 _input_limits[0][1],
                 ". Old: ",
                 MetaPhysicL::raw_value(_mobile_old),
                 " increment: ",
                 MetaPhysicL::raw_value(_mobile_dislocation_increment),
                 ". Cutting timestep");
    if (_immobile_dislocations[_qp] < _input_limits[1][0] ||
        _immobile_dislocations[_qp] > _input_limits[1][1])
      mooseError("In ",
                 _name,
                 ": immobile dislocations (",
                 MetaPhysicL::raw_value(_immobile_dislocations[_qp]),
                 ") is out side of its limits (",
                 _input_limits[1][0],
                 " - ",
                 _input_limits[1][1],
                 ". Old: ",
                 MetaPhysicL::raw_value(_immobile_old),
                 " increment: ",
                 MetaPhysicL::raw_value(_immobile_dislocation_increment),
                 ". Cutting timestep");
  }

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
