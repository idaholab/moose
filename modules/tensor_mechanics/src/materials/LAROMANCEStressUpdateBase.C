//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LAROMANCEStressUpdateBase.h"
#include "Function.h"
#include "MathUtils.h"

template <bool is_ad>
InputParameters
LAROMANCEStressUpdateBaseTempl<is_ad>::validParams()
{
  InputParameters params = RadialReturnCreepStressUpdateBaseTempl<is_ad>::validParams();
  params.addClassDescription(
      "Calculates the effective creep strain based on the rates predicted by a material "
      "specific Los Alamos Reduced Order Model derived from a Visco-Plastic Self Consistent "
      "calculations.");

  params.addRequiredCoupledVar("temperature", "The coupled temperature (K)");
  params.addParam<MaterialPropertyName>("environmental_factor",
                                        "Optional coupled environmental factor");

  MooseEnum error_limit_behavior("ERROR WARN IGNORE EXCEPTION", "EXCEPTION");
  params.addParam<MooseEnum>("cell_input_window_failure",
                             error_limit_behavior,
                             "What to do if cell dislocation concentration is outside the global "
                             "window of applicability.");
  params.addParam<MooseEnum>("wall_input_window_failure",
                             error_limit_behavior,
                             "What to do if wall dislocation concentration is outside the "
                             "global window of applicability.");
  params.addParam<MooseEnum>(
      "old_strain_input_window_failure",
      error_limit_behavior,
      "What to do if old strain is outside the global window of applicability.");

  MooseEnum extrapolated_limit_behavior("ERROR WARN IGNORE EXCEPTION EXTRAPOLATE", "EXTRAPOLATE");
  params.addParam<MooseEnum>("stress_input_window_failure",
                             extrapolated_limit_behavior,
                             "What to do if stress is outside the global window of applicability.");
  params.addParam<MooseEnum>(
      "temperature_input_window_failure",
      extrapolated_limit_behavior,
      "What to do if temperature is outside the global window of applicability.");
  params.addParam<MooseEnum>(
      "environment_input_window_failure",
      extrapolated_limit_behavior,
      "What to do if environmental factor is outside the global window of applicability.");

  params.addRequiredRangeCheckedParam<Real>(
      "initial_cell_dislocation_density",
      "initial_cell_dislocation_density >= 0.0",
      "Initial density of cell (glissile) dislocations (1/m^2)");
  params.addRangeCheckedParam<Real>(
      "max_relative_cell_dislocation_increment",
      0.5,
      "max_relative_cell_dislocation_increment > 0.0",
      "Maximum increment of density of cell (glissile) dislocations.");

  params.addRequiredRangeCheckedParam<Real>(
      "initial_wall_dislocation_density",
      "initial_wall_dislocation_density >= 0.0",
      "wall (locked) dislocation density initial value (1/m^2).");
  params.addRangeCheckedParam<Real>(
      "max_relative_wall_dislocation_increment",
      0.5,
      "max_relative_wall_dislocation_increment > 0.0",
      "Maximum increment of wall (locked) dislocation density initial value (1/m^2).");

  params.addParam<bool>("verbose", false, "Flag to output verbose information.");

  params.addParam<FunctionName>(
      "cell_dislocation_density_forcing_function",
      "Optional forcing function for wall dislocation. If provided, the wall dislocation "
      "density will be reset to the function value at the beginning of the timestep. Used for "
      "testing purposes only.");
  params.addParam<FunctionName>(
      "wall_dislocation_density_forcing_function",
      "Optional forcing function for wall dislocation. If provided, the wall dislocation "
      "density will be reset to the function value at the beginning of the timestep. Used for "
      "testing purposes only.");
  params.addParam<FunctionName>(
      "old_creep_strain_forcing_function",
      "Optional forcing function for the creep strain from the previous timestep. If provided, "
      "the old creep strain will be reset to the function value at the beginning of the "
      "timestep. Used for testing purposes only.");
  params.addParamNamesToGroup(
      "cell_dislocation_density_forcing_function wall_dislocation_density_forcing_function "
      "old_creep_strain_forcing_function",
      "Advanced");

  return params;
}

template <bool is_ad>
LAROMANCEStressUpdateBaseTempl<is_ad>::LAROMANCEStressUpdateBaseTempl(
    const InputParameters & parameters)
  : RadialReturnCreepStressUpdateBaseTempl<is_ad>(parameters),
    _temperature(this->template coupledGenericValue<is_ad>("temperature")),
    _environmental(
        this->isParamValid("environmental_factor")
            ? &this->template getGenericMaterialProperty<Real, is_ad>("environmental_factor")
            : nullptr),
    _window_failure(_environmental ? 6 : 5),

    _verbose(this->template getParam<bool>("verbose")),
    _cell_dislocations(
        this->template declareGenericProperty<Real, is_ad>(this->_base_name + "cell_dislocations")),
    _cell_dislocations_old(
        this->template getMaterialPropertyOld<Real>(this->_base_name + "cell_dislocations")),
    _initial_cell_dislocations(this->template getParam<Real>("initial_cell_dislocation_density")),
    _max_cell_increment(this->template getParam<Real>("max_relative_cell_dislocation_increment")),
    _cell_function(this->isParamValid("cell_dislocation_density_forcing_function")
                       ? &this->getFunction("cell_dislocation_density_forcing_function")
                       : NULL),
    _cell_dislocation_increment(0.0),
    _wall_dislocations(
        this->template declareGenericProperty<Real, is_ad>(this->_base_name + "wall_dislocations")),
    _wall_dislocations_old(
        this->template getMaterialPropertyOld<Real>(this->_base_name + "wall_dislocations")),
    _initial_wall_dislocations(this->template getParam<Real>("initial_wall_dislocation_density")),
    _max_wall_increment(this->template getParam<Real>("max_relative_wall_dislocation_increment")),
    _wall_function(this->isParamValid("wall_dislocation_density_forcing_function")
                       ? &this->getFunction("wall_dislocation_density_forcing_function")
                       : NULL),
    _wall_dislocation_increment(0.0),
    _cell_input_index(0),
    _wall_input_index(1),
    _stress_input_index(2),
    _old_strain_input_index(3),
    _temperature_input_index(4),
    _environmental_input_index(5),
    _cell_output_index(0),
    _wall_output_index(1),
    _strain_output_index(2),

    _creep_strain_old_forcing_function(this->isParamValid("old_creep_strain_forcing_function")
                                           ? &this->getFunction("old_creep_strain_forcing_function")
                                           : NULL),

    _creep_rate(
        this->template declareGenericProperty<Real, is_ad>(this->_base_name + "creep_rate")),
    _cell_rate(this->template declareGenericProperty<Real, is_ad>(this->_base_name +
                                                                  "cell_dislocation_rate")),
    _wall_rate(this->template declareGenericProperty<Real, is_ad>(this->_base_name +
                                                                  "wall_dislocation_rate")),
    _extrapolation(this->template declareProperty<Real>("ROM_extrapolation")),

    _derivative(0.0),
    _old_input_values(3)
{
  this->_check_range = true;

  _window_failure[_cell_input_index] =
      parameters.get<MooseEnum>("cell_input_window_failure").getEnum<WindowFailure>();
  _window_failure[_wall_input_index] =
      parameters.get<MooseEnum>("wall_input_window_failure").getEnum<WindowFailure>();
  _window_failure[_stress_input_index] =
      parameters.get<MooseEnum>("stress_input_window_failure").getEnum<WindowFailure>();
  _window_failure[_old_strain_input_index] =
      parameters.get<MooseEnum>("old_strain_input_window_failure").getEnum<WindowFailure>();
  _window_failure[_temperature_input_index] =
      parameters.get<MooseEnum>("temperature_input_window_failure").getEnum<WindowFailure>();
  if (_environmental)
    _window_failure[_environmental_input_index] =
        parameters.get<MooseEnum>("environment_input_window_failure").getEnum<WindowFailure>();
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::initialSetup()
{
  // Pull in relevant ROM information and do sanity checks
  _transform = getTransform();
  _transform_coefs = getTransformCoefs();
  _input_limits = getInputLimits();
  _normalization_limits = getNormalizationLimits();
  _coefs = getCoefs();
  _tiling = getTilings();

  _num_tiles = _transform.size();
  if (!_num_tiles)
    mooseError("In ", this->_name, ": No tiles detected. Double check your ROM input");

  _num_outputs = _transform[0].size();
  if (_num_outputs != 3)
    mooseError("In ",
               this->_name,
               ": ",
               _num_outputs,
               " outputs detected. Three and only three outputs are currently supported.");

  _num_inputs = _transform[0][0].size();
  if (_num_inputs != 5 && _num_inputs != 6)
    mooseError("In ",
               this->_name,
               ": ",
               _num_inputs,
               " inputs detected. Only five or six inputs currently supported.");
  if (_num_inputs == 6 && !_environmental)
    this->template paramError(
        "environmental",
        "Number of ROM inputs indicate environmental factor is required to be coupled.");
  if (_num_inputs != 6 && _environmental)
    this->template paramError(
        "environmental",
        "Number of ROM inputs indicate environmental factor is not implemented, but "
        "environmental factor coupled.");

  bool correct_shape = true;
  if (_transform.size() != _num_tiles || _transform_coefs.size() != _num_tiles ||
      _input_limits.size() != _num_tiles || _normalization_limits.size() != _num_tiles ||
      _coefs.size() != _num_tiles)
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
    if (_input_limits[t].size() != _num_inputs || _normalization_limits[t].size() != _num_inputs)
      correct_shape = false;
    for (unsigned int i = 0; i < _num_inputs; ++i)
      if (_input_limits[t][i].size() != 2 || _normalization_limits[t][i].size() != 2)
        correct_shape = false;
  }

  if (!correct_shape)
    mooseError("In ", this->_name, ": ROM data is not the right shape.");

  unsigned int tiles_check = 1;
  for (unsigned int i = 0; i < _num_inputs; ++i)
    tiles_check *= _tiling[i];
  if (tiles_check != _num_tiles)
    mooseError("In ",
               this->_name,
               ": Product of getTiles (",
               tiles_check,
               ") is not the same as num_tiles (",
               _num_tiles,
               ")");

  _degree = std::pow(_num_coefs, 1.0 / _num_inputs);
  if (!_degree || _degree > 4)
    mooseError("In ", this->_name, ": degree must be 1, 2, 3 or 4.");

  // Check input limits and find global limits
  _global_limits.resize(_num_inputs, std::vector<Real>(2));
  for (unsigned int i = 0; i < _num_inputs; ++i)
  {
    _global_limits[i][0] = std::numeric_limits<Real>::max();
    _global_limits[i][1] = 0.0;
    for (unsigned int t = 0; t < _num_tiles; ++t)
    {
      if (_input_limits[t][i][0] >= _input_limits[t][i][1])
        mooseError("In ", this->_name, ": Input limits are ordered incorrectly");
      _global_limits[i][0] = std::min(_global_limits[i][0], _input_limits[t][i][0]);
      _global_limits[i][1] = std::max(_global_limits[i][1], _input_limits[t][i][1]);
    }
  }

  // Precompute helper containers
  _transformed_normalization_limits = getTransformedLimits(_normalization_limits);
  _makeframe_helper = getMakeFrameHelper();

  // Prepare containers
  _input_values.resize(_num_inputs);
  _precomputed_vals.resize(_num_tiles, std::vector<GenericReal<is_ad>>(_num_coefs));
  _rom_inputs.resize(_num_tiles, std::vector<GenericReal<is_ad>>(_num_inputs));
  _polynomial_inputs.resize(_num_tiles,
                            std::vector<std::vector<GenericReal<is_ad>>>(
                                _num_inputs, std::vector<GenericReal<is_ad>>(_degree)));
  _non_stress_weights.resize(_num_tiles);
  _weights.resize(_num_tiles);

  if (_verbose)
  {
    Moose::err << "ROM model info: " << this->_name << "\n";
    Moose::err << " number of tiles: " << _num_tiles << "\n";
    Moose::err << " number of outputs: " << _num_outputs << "\n";
    Moose::err << " number of inputs: " << _num_inputs << "\n";
    Moose::err << " degree (max Legendre degree + constant): " << _degree << "\n";
    Moose::err << " number of coefficients: " << _num_coefs << "\n";
    Moose::err << " Global limits:\n  cell dislocations (" << _global_limits[_cell_input_index][0]
               << " - " << _global_limits[_cell_input_index][1] << ")\n";
    Moose::err << "  wall dislocations (" << _global_limits[_wall_input_index][0] << " - "
               << _global_limits[_wall_input_index][1] << ")\n";
    Moose::err << "  Stress (" << _global_limits[_stress_input_index][0] << " - "
               << _global_limits[_stress_input_index][1] << ")\n";
    Moose::err << "  Old strain (" << _global_limits[_old_strain_input_index][0] << " - "
               << _global_limits[_old_strain_input_index][1] << ")\n";
    Moose::err << "  Temperature (" << _global_limits[_temperature_input_index][0] << " - "
               << _global_limits[_temperature_input_index][1] << ")\n";
    if (_environmental)
      Moose::err << "  Environmental factor (" << _global_limits[_environmental_input_index][0]
                 << " - " << _global_limits[_environmental_input_index][1] << ")\n";
    Moose::err << std::endl;
  }
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::initQpStatefulProperties()
{
  _cell_dislocations[_qp] = _initial_cell_dislocations;
  _wall_dislocations[_qp] = _initial_wall_dislocations;

  RadialReturnCreepStressUpdateBaseTempl<is_ad>::initQpStatefulProperties();
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEStressUpdateBaseTempl<is_ad>::maximumPermissibleValue(
    const GenericReal<is_ad> & effective_trial_stress) const
{
  // Make maximum allowed scalar a little bit less than the deformation that would reduce the
  // trial stress to zero. This prevents negative trial stresses.
  return effective_trial_stress / this->_three_shear_modulus * 0.999999;
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::computeStressInitialize(
    const GenericReal<is_ad> & effective_trial_stress,
    const GenericRankFourTensor<is_ad> & elasticity_tensor)
{
  RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeStressInitialize(effective_trial_stress,
                                                                         elasticity_tensor);

  // Prepare old values
  _old_input_values[_cell_output_index] =
      _cell_function ? _cell_function->value(_t, _q_point[_qp]) : _cell_dislocations_old[_qp];
  _old_input_values[_wall_output_index] =
      _wall_function ? _wall_function->value(_t, _q_point[_qp]) : _wall_dislocations_old[_qp];
  _old_input_values[_strain_output_index] =
      _creep_strain_old_forcing_function
          ? _creep_strain_old_forcing_function->value(_t, _q_point[_qp])
          : std::sqrt(this->_creep_strain_old[_qp].doubleContraction(this->_creep_strain_old[_qp]) /
                      1.5);

  // Prepare input
  _input_values[_cell_input_index] = _old_input_values[_cell_output_index];
  _input_values[_wall_input_index] = _old_input_values[_wall_output_index];
  _input_values[_stress_input_index] = effective_trial_stress * 1.0e-6;
  _input_values[_old_strain_input_index] = _old_input_values[_strain_output_index];
  _input_values[_temperature_input_index] = _temperature[_qp];
  if (_environmental)
    _input_values[_environmental_input_index] = (*_environmental)[_qp];

  // Determine tile mixing weight and check to see if input is in range
  std::fill(_non_stress_weights.begin(), _non_stress_weights.end(), 1.0);
  for (unsigned int i = 0; i < _num_inputs; i++)
  {
    if (i != _stress_input_index)
    {
      computeTileWeight(_non_stress_weights, _input_values[i], i);
      checkInputWindow(_input_values[i], _window_failure[i], _global_limits[i]);
    }
  }

  // Precompute transformed input and prebuild polynomials for inputs other than strain
  precomputeROM(_strain_output_index);
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::computeTileWeight(std::vector<GenericReal<is_ad>> & weights,
                                                         GenericReal<is_ad> & input,
                                                         const unsigned int in_index,
                                                         const bool derivative)
{
  for (unsigned int t = 0; t < _num_tiles; ++t)
  {
    // If tiling is not available for this input
    if (_tiling[in_index] < 2)
    {
      // Extrapolate if input limit of the current tile is the global min
      if (_window_failure[in_index] == WindowFailure::EXTRAPOLATE &&
          _input_limits[t][in_index][0] == _global_limits[in_index][0] &&
          input < _input_limits[t][in_index][0])
      {
        if (derivative)
          weights[t] *= -sigmoid(0.0, _global_limits[in_index][0], input, true);
        else
          weights[t] *= (1.0 - sigmoid(0.0, _global_limits[in_index][0], input));
        input = _global_limits[in_index][0];
      }
    }
    else
    {
      // If input is within a specfic tile's window of applicability
      if (input >= _input_limits[t][in_index][0] && input < _input_limits[t][in_index][1])
      {
        // Flag to ensure weights are applied only once
        bool overlap = false;
        for (unsigned int tt = 0; tt < _num_tiles; ++tt)
        {
          if (!overlap && t != tt)
          {
            // If input is within another tile's window of applicability, i.e. tiled
            if ((_input_limits[t][in_index][0] != _input_limits[tt][in_index][0] ||
                 _input_limits[t][in_index][1] != _input_limits[tt][in_index][1]) &&
                input >= _input_limits[tt][in_index][0] && input < _input_limits[tt][in_index][1])
            {
              overlap = true;
              // If current tile is below the second tile's window of applicability
              if (_input_limits[t][in_index][0] < _input_limits[tt][in_index][0] &&
                  _input_limits[t][in_index][1] > _input_limits[tt][in_index][0])
              {
                weights[t] *= sigmoid(_input_limits[tt][in_index][0],
                                      _input_limits[t][in_index][1],
                                      input,
                                      derivative);
              }
              // If current tile is above the second tile's window of applicability
              else if (_input_limits[t][in_index][0] > _input_limits[tt][in_index][0] &&
                       _input_limits[t][in_index][0] < _input_limits[tt][in_index][1])
              {
                if (derivative)
                  weights[t] *= -sigmoid(_input_limits[t][in_index][0],
                                         _input_limits[tt][in_index][1],
                                         input,
                                         derivative);
                else
                  weights[t] *= (1.0 - sigmoid(_input_limits[t][in_index][0],
                                               _input_limits[tt][in_index][1],
                                               input));
              }
            }
          }
        }
        // If not overlapping, weight = 1, and there is no derivative
        if (!overlap && derivative)
          weights[t] *= 0.0;
      }
      // If input is outside window of applicability, weight is zero
      else
        weights[t] *= 0.0;
    }
  }
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::checkInputWindow(const GenericReal<is_ad> & input,
                                                        const WindowFailure behavior,
                                                        const std::vector<Real> & global_limits)
{
  if (behavior == WindowFailure::IGNORE || behavior == WindowFailure::EXTRAPOLATE)
    return;

  if (input < global_limits[0] || input > global_limits[1])
  {
    std::stringstream msg;
    msg << "In " << this->_name << ": input parameter with value (" << MetaPhysicL::raw_value(input)
        << ") is out of global range (" << global_limits[0] << " - " << global_limits[1] << ")";

    switch (behavior)
    {
      case WindowFailure::WARN:
        mooseWarning(msg.str());
        break;
      case WindowFailure::EXCEPTION:
        mooseException(msg.str());
        break;
      case WindowFailure::ERROR:
        mooseError(msg.str());
        break;
      default:
        mooseError("Internal enum error");
    }
  }
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEStressUpdateBaseTempl<is_ad>::computeResidual(
    const GenericReal<is_ad> & effective_trial_stress, const GenericReal<is_ad> & scalar)
{
  // Update new stress
  auto trial_stress_mpa = effective_trial_stress * 1.0e-6;
  GenericReal<is_ad> dtrial_stress_dscalar = 0.0;

  // Update stress if strain is being applied, i.e. non-testing simulation
  if (this->_apply_strain)
  {
    trial_stress_mpa -= this->_three_shear_modulus * scalar * 1.0e-6;
    dtrial_stress_dscalar -= this->_three_shear_modulus * 1.0e-6;
  }
  _input_values[_stress_input_index] = trial_stress_mpa;

  // Update weights with new stress
  _weights = _non_stress_weights;
  computeTileWeight(_weights, _input_values[_stress_input_index], _stress_input_index);
  auto dweights_dstress = _non_stress_weights;
  computeTileWeight(
      dweights_dstress, _input_values[_stress_input_index], _stress_input_index, true);

  // Check window with new stress
  checkInputWindow(_input_values[_stress_input_index],
                   _window_failure[_stress_input_index],
                   _global_limits[_stress_input_index]);

  // Save extrapolation as a material proeprty in order quantify adequate tiling range
  _extrapolation[_qp] = 0.0;
  for (unsigned int t = 0; t < _num_tiles; ++t)
    _extrapolation[_qp] += MetaPhysicL::raw_value(_weights[t]);

  GenericReal<is_ad> total_rom_effective_strain_inc = 0.0;
  GenericReal<is_ad> dtotal_rom_effective_strain_inc_dstress = 0.0;

  // Run ROM if all values are within windows.
  for (unsigned int t = 0; t < _num_tiles; ++t)
  {
    if (_weights[t])
    {
      const GenericReal<is_ad> rom = computeROM(t, _strain_output_index);
      total_rom_effective_strain_inc += _weights[t] * rom;
      dtotal_rom_effective_strain_inc_dstress +=
          _weights[t] * computeROM(t, _strain_output_index, true) + dweights_dstress[t] * rom;
    }
  }

  if (_verbose)
  {
    GenericReal<is_ad> environmental = 0.0;
    if (_environmental)
      environmental = (*_environmental)[_qp];
    Moose::err << "Verbose information from " << this->_name << ": \n";
    Moose::err << "  dt: " << _dt << "\n";
    Moose::err << "  old cell disl: " << _old_input_values[_cell_output_index] << "\n";
    Moose::err << "  old wall disl: " << _old_input_values[_wall_output_index] << "\n";
    Moose::err << "  initial stress (MPa): "
               << MetaPhysicL::raw_value(effective_trial_stress) * 1.0e-6 << "\n";
    Moose::err << "  temperature: " << MetaPhysicL::raw_value(_temperature[_qp]) << "\n";
    Moose::err << "  environmental factor: " << MetaPhysicL::raw_value(environmental) << "\n";
    Moose::err << "  calculated scalar strain value: " << MetaPhysicL::raw_value(scalar) << "\n";
    Moose::err << "  trial stress into rom (MPa): " << MetaPhysicL::raw_value(trial_stress_mpa)
               << "\n";
    Moose::err << "  old effective strain: " << _old_input_values[_strain_output_index] << "\n";
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

  _creep_rate[_qp] = total_rom_effective_strain_inc / _dt;
  _derivative = dtotal_rom_effective_strain_inc_dstress * dtrial_stress_dscalar - 1.0;

  if (!this->_apply_strain)
  {
    if (_verbose)
      Moose::err << "    Strain not applied due to apply_strain input parameter!" << std::endl;
    _derivative = 1.0;
    return 0.0;
  }
  return total_rom_effective_strain_inc - scalar;
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::precomputeROM(const unsigned out_index)
{
  for (unsigned int t = 0; t < _num_tiles; ++t)
  {
    // Only precompute for tiles that don't have zero weight
    if (_non_stress_weights[t])
    {
      for (unsigned int i = 0; i < _num_inputs; ++i)
      {
        if (i != _stress_input_index)
        {
          _rom_inputs[t][i] = normalizeInput(_input_values[i],
                                             _transform[t][out_index][i],
                                             _transform_coefs[t][out_index][i],
                                             _transformed_normalization_limits[t][out_index][i]);
          buildPolynomials(_rom_inputs[t][i], _polynomial_inputs[t][i]);
        }
      }
      precomputeValues(_coefs[t][out_index], _polynomial_inputs[t], _precomputed_vals[t]);
    }
  }
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEStressUpdateBaseTempl<is_ad>::computeROM(const unsigned int t,
                                                  const unsigned out_index,
                                                  const bool derivative)
{
  // Update due to new stress
  _rom_inputs[t][_stress_input_index] =
      normalizeInput(_input_values[_stress_input_index],
                     _transform[t][out_index][_stress_input_index],
                     _transform_coefs[t][out_index][_stress_input_index],
                     _transformed_normalization_limits[t][out_index][_stress_input_index]);
  buildPolynomials(_rom_inputs[t][_stress_input_index], _polynomial_inputs[t][_stress_input_index]);

  // Compute ROM values
  const GenericReal<is_ad> rom_output = computeValues(_precomputed_vals[t], _polynomial_inputs[t]);

  // Return converted output if not derivative
  if (!derivative)
    return convertOutput(_old_input_values, rom_output, out_index);

  const GenericReal<is_ad> drom_input =
      normalizeInput(_input_values[_stress_input_index],
                     _transform[t][out_index][_stress_input_index],
                     _transform_coefs[t][out_index][_stress_input_index],
                     _transformed_normalization_limits[t][out_index][_stress_input_index],
                     derivative);

  std::vector<GenericReal<is_ad>> dpolynomial_inputs(_degree, 0.0);
  buildPolynomials(_rom_inputs[t][_stress_input_index], dpolynomial_inputs, drom_input, derivative);

  const GenericReal<is_ad> drom_output =
      computeValues(_precomputed_vals[t], _polynomial_inputs[t], dpolynomial_inputs, derivative);

  return convertOutput(_old_input_values, rom_output, out_index, drom_output, derivative);
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEStressUpdateBaseTempl<is_ad>::normalizeInput(const GenericReal<is_ad> & input,
                                                      const ROMInputTransform transform,
                                                      const Real transform_coef,
                                                      const std::vector<Real> & transformed_limits,
                                                      const bool derivative)
{
  GenericReal<is_ad> x = input;
  convertValue(x, transform, transform_coef, derivative);

  // transformed_limits[2] = transformed_limits[1] - transformed_limits[0]
  if (derivative)
    return x / transformed_limits[2];
  else
    return (x - transformed_limits[0]) / transformed_limits[2] - 1.0;
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::buildPolynomials(
    const GenericReal<is_ad> & rom_input,
    std::vector<GenericReal<is_ad>> & polynomial_inputs,
    const GenericReal<is_ad> & drom_input,
    const bool derivative)
{
  for (unsigned int d = 0; d < _degree; ++d)
  {
    polynomial_inputs[d] = computePolynomial(rom_input, d);
    if (derivative)
      polynomial_inputs[d] = drom_input * computePolynomial(rom_input, d, derivative);
  }
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::precomputeValues(
    const std::vector<Real> & coefs,
    const std::vector<std::vector<GenericReal<is_ad>>> & polynomial_inputs,
    std::vector<GenericReal<is_ad>> & precomputed)
{
  for (unsigned int c = 0; c < _num_coefs; ++c)
  {
    precomputed[c] = coefs[c];
    for (unsigned int i = 0; i < _num_inputs; ++i)
      if (i != _stress_input_index)
        precomputed[c] *= polynomial_inputs[i][_makeframe_helper[c + _num_coefs * i]];
  }
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEStressUpdateBaseTempl<is_ad>::computeValues(
    const std::vector<GenericReal<is_ad>> & precomputed,
    const std::vector<std::vector<GenericReal<is_ad>>> & polynomial_inputs,
    const std::vector<GenericReal<is_ad>> & dpolynomial_inputs,
    const bool derivative)
{
  GenericReal<is_ad> rom_output = 0.0;
  for (unsigned int c = 0; c < _num_coefs; ++c)
  {
    if (!derivative)
      rom_output += precomputed[c] *
                    polynomial_inputs[_stress_input_index]
                                     [_makeframe_helper[c + _num_coefs * _stress_input_index]];
    else
      rom_output += precomputed[c] *
                    dpolynomial_inputs[_makeframe_helper[c + _num_coefs * _stress_input_index]];
  }
  return rom_output;
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEStressUpdateBaseTempl<is_ad>::convertOutput(const std::vector<Real> & old_input_values,
                                                     const GenericReal<is_ad> & rom_output,
                                                     const unsigned out_index,
                                                     const GenericReal<is_ad> & drom_output,
                                                     const bool derivative)
{
  if (out_index == _strain_output_index)
  {
    if (derivative)
      return std::exp(rom_output) * _dt * drom_output;
    else
      return std::exp(rom_output) * _dt;
  }

  if (derivative)
    return 0.0;

  GenericReal<is_ad> expout = std::exp(rom_output);
  mooseAssert(expout > 0.0, "ROM calculated strain increment is not strictly positive");

  const Real rom_strain_cutoff_value = romStrainCutoff();
  if (expout > rom_strain_cutoff_value)
    expout -= rom_strain_cutoff_value;
  else
    expout = -rom_strain_cutoff_value * rom_strain_cutoff_value / expout + rom_strain_cutoff_value;

  return -expout * old_input_values[out_index] * _dt;
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEStressUpdateBaseTempl<is_ad>::computePolynomial(const GenericReal<is_ad> & value,
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

template <bool is_ad>
std::vector<std::vector<std::vector<std::vector<Real>>>>
LAROMANCEStressUpdateBaseTempl<is_ad>::getTransformedLimits(
    const std::vector<std::vector<std::vector<Real>>> limits)
{
  std::vector<std::vector<std::vector<std::vector<Real>>>> transformed_limits(
      _num_tiles,
      std::vector<std::vector<std::vector<Real>>>(
          _num_outputs, std::vector<std::vector<Real>>(_num_inputs, std::vector<Real>(3))));

  for (unsigned int t = 0; t < _num_tiles; ++t)
  {
    for (unsigned int o = 0; o < _num_outputs; ++o)
    {
      for (unsigned int i = 0; i < _num_inputs; ++i)
      {
        for (unsigned int k = 0; k < 2; ++k)
        {
          transformed_limits[t][o][i][k] = limits[t][i][k];
          convertValue(
              transformed_limits[t][o][i][k], _transform[t][o][i], _transform_coefs[t][o][i]);
        }
        transformed_limits[t][o][i][2] =
            (transformed_limits[t][o][i][1] - transformed_limits[t][o][i][0]) / 2.0;
      }
    }
  }

  return transformed_limits;
}

template <bool is_ad>
std::vector<unsigned int>
LAROMANCEStressUpdateBaseTempl<is_ad>::getMakeFrameHelper() const
{
  std::vector<unsigned int> v(_num_coefs * _num_inputs);

  for (unsigned int numcoeffs = 0; numcoeffs < _num_coefs; ++numcoeffs)
    for (unsigned int invar = 0; invar < _num_inputs; ++invar)
      v[numcoeffs + _num_coefs * invar] = numcoeffs / MathUtils::pow(_degree, invar) % _degree;

  return v;
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::computeStressFinalize(
    const GenericRankTwoTensor<is_ad> & plastic_strain_increment)
{
  _cell_dislocation_increment = 0.0;
  _wall_dislocation_increment = 0.0;

  precomputeROM(_cell_output_index);
  for (unsigned int t = 0; t < _num_tiles; ++t)
    if (_weights[t])
      _cell_dislocation_increment += _weights[t] * computeROM(t, _cell_output_index);

  precomputeROM(_wall_output_index);
  for (unsigned int t = 0; t < _num_tiles; ++t)
    if (_weights[t])
      _wall_dislocation_increment += _weights[t] * computeROM(t, _wall_output_index);

  _cell_rate[_qp] = _cell_dislocation_increment / _dt;
  _wall_rate[_qp] = _wall_dislocation_increment / _dt;
  _cell_dislocations[_qp] = _old_input_values[_cell_output_index] + _cell_dislocation_increment;
  _wall_dislocations[_qp] = _old_input_values[_wall_output_index] + _wall_dislocation_increment;

  // Prevent the ROM from calculating and proceeding with negative dislocations
  if ((_cell_dislocations[_qp] < 0.0 || _wall_dislocations[_qp] < 0.0) && (this->_apply_strain))
  {
    const Real negative_cell_dislocations = MetaPhysicL::raw_value(_cell_dislocations[_qp]);
    const Real negative_wall_dislocations = MetaPhysicL::raw_value(_wall_dislocations[_qp]);
    _cell_dislocations[_qp] = _old_input_values[_cell_output_index];
    _wall_dislocations[_qp] = _old_input_values[_wall_output_index];
    mooseException("The negative values of the cell dislocation density, ",
                   negative_cell_dislocations,
                   ", and/or wall dislocation density, ",
                   negative_wall_dislocations,
                   ". Cutting timestep.");
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
                            MetaPhysicL::raw_value(this->_creep_strain[_qp].doubleContraction(
                                this->_creep_strain[_qp])))
               << "\n";
    Moose::err << "  creep rate: " << MetaPhysicL::raw_value(_creep_rate[_qp]) << "\n";
    Moose::err << "  cell dislocation rate: " << MetaPhysicL::raw_value(_cell_rate[_qp]) << "\n";
    Moose::err << "  wall dislocation rate: " << MetaPhysicL::raw_value(_wall_rate[_qp]) << "\n";
    Moose::err << "  new cell dislocations: " << MetaPhysicL::raw_value(_cell_dislocations[_qp])
               << "\n";
    Moose::err << "  new wall dislocations: " << MetaPhysicL::raw_value(_wall_dislocations[_qp])
               << "\n"
               << std::endl;
  }

  RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeStressFinalize(plastic_strain_increment);
}

template <bool is_ad>
Real
LAROMANCEStressUpdateBaseTempl<is_ad>::computeTimeStepLimit()
{
  Real limited_dt = RadialReturnStressUpdateTempl<is_ad>::computeTimeStepLimit();

  Real cell_strain_inc = std::abs(MetaPhysicL::raw_value(_cell_dislocation_increment));
  if (cell_strain_inc && _old_input_values[_cell_output_index])
    limited_dt = std::min(limited_dt,
                          _dt * _max_cell_increment * _old_input_values[_cell_output_index] /
                              cell_strain_inc);
  Real wall_strain_inc = std::abs(MetaPhysicL::raw_value(_wall_dislocation_increment));
  if (wall_strain_inc && _old_input_values[_wall_output_index])
    limited_dt = std::min(limited_dt,
                          _dt * _max_wall_increment * _old_input_values[_wall_output_index] /
                              wall_strain_inc);

  return limited_dt;
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEStressUpdateBaseTempl<is_ad>::sigmoid(const Real lower,
                                               const Real upper,
                                               const GenericReal<is_ad> & val,
                                               const bool derivative)
{
  GenericReal<is_ad> x = (val - lower) / (upper - lower);
  x = 2.0 * x - 1.0;
  GenericReal<is_ad> plus = std::exp(-2.0 / (1.0 + x));
  GenericReal<is_ad> minus = std::exp(-2.0 / (1.0 - x));
  if (!derivative)
    return 1.0 - plus / (plus + minus);

  GenericReal<is_ad> dplus = plus * 2.0 / Utility::pow<2>(1.0 + x);
  GenericReal<is_ad> dminus = -minus * 2.0 / Utility::pow<2>(1.0 - x);

  return (plus * dminus - dplus * minus) / Utility::pow<2>(plus + minus) * 2.0 / (upper - lower);
}

template class LAROMANCEStressUpdateBaseTempl<false>;
template class LAROMANCEStressUpdateBaseTempl<true>;
