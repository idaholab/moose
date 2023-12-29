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
#include "MooseUtils.h"
#include "MooseRandom.h"
#include "Units.h"

#include <iostream>

registerMooseObjectAliased("TensorMechanicsApp",
                           LAROMANCEStressUpdateBase,
                           "LAROMANCEStressUpdate");
registerMooseObjectAliased("TensorMechanicsApp",
                           ADLAROMANCEStressUpdateBase,
                           "ADLAROMANCEStressUpdate");

template <bool is_ad>
InputParameters
LAROMANCEStressUpdateBaseTempl<is_ad>::validParams()
{
  InputParameters params = RadialReturnCreepStressUpdateBaseTempl<is_ad>::validParams();
  params.addClassDescription("Base class to calculate the effective creep strain based on the "
                             "rates predicted by a material  specific Los Alamos Reduced Order "
                             "Model derived from a Visco-Plastic Self Consistent calculations.");

  params.addRequiredCoupledVar("temperature", "The coupled temperature (K)");
  params.addParam<MaterialPropertyName>("environmental_factor",
                                        "Optional coupled environmental factor");

  MooseEnum error_lower_limit_behavior("ERROR EXCEPTION WARN IGNORE DONTHING USELIMIT",
                                       "EXCEPTION");
  // Only allow ERROR and EXCEPTION on upper bounds
  MooseEnum error_upper_limit_behavior("ERROR EXCEPTION", "EXCEPTION");
  params.addParam<MooseEnum>(
      "cell_input_window_low_failure",
      error_lower_limit_behavior,
      "What to do if cell dislocation concentration is outside the lower global "
      "window of applicability.");
  params.addParam<MooseEnum>(
      "cell_input_window_high_failure",
      error_upper_limit_behavior,
      "What to do if cell dislocation concentration is outside the upper global "
      "window of applicability.");
  params.addParam<MooseEnum>("wall_input_window_low_failure",
                             error_lower_limit_behavior,
                             "What to do if wall dislocation concentration is outside the "
                             "lower global window of applicability.");
  params.addParam<MooseEnum>("wall_input_window_high_failure",
                             error_upper_limit_behavior,
                             "What to do if wall dislocation concentration is outside the "
                             "upper global window of applicability.");
  params.addParam<MooseEnum>(
      "old_strain_input_window_low_failure",
      error_lower_limit_behavior,
      "What to do if old strain is outside the lower global window of applicability.");
  params.addParam<MooseEnum>(
      "old_strain_input_window_high_failure",
      error_upper_limit_behavior,
      "What to do if old strain is outside the upper global window of applicability.");

  MooseEnum extrapolated_lower_limit_behavior(
      "ERROR EXCEPTION WARN IGNORE DONOTHING USELIMIT EXTRAPOLATE", "EXTRAPOLATE");
  params.addParam<MooseEnum>(
      "stress_input_window_low_failure",
      extrapolated_lower_limit_behavior,
      "What to do if stress is outside the lower global window of applicability.");
  params.addParam<MooseEnum>(
      "stress_input_window_high_failure",
      error_upper_limit_behavior,
      "What to do if stress is outside the upper global window of applicability.");
  params.addParam<MooseEnum>(
      "temperature_input_window_low_failure",
      extrapolated_lower_limit_behavior,
      "What to do if temperature is outside the lower global window of applicability.");
  params.addParam<MooseEnum>(
      "temperature_input_window_high_failure",
      error_upper_limit_behavior,
      "What to do if temperature is outside the upper global window of applicability.");
  params.addParam<MooseEnum>(
      "environment_input_window_low_failure",
      extrapolated_lower_limit_behavior,
      "What to do if environmental factor is outside the lower global window of applicability.");
  params.addParam<MooseEnum>(
      "environment_input_window_high_failure",
      error_upper_limit_behavior,
      "What to do if environmental factor is outside the upper global window of applicability.");

  params.addRequiredRangeCheckedParam<Real>(
      "initial_cell_dislocation_density",
      "initial_cell_dislocation_density >= 0.0",
      "Initial density of cell (glissile) dislocations (1/m^2)");
  params.addRangeCheckedParam<Real>(
      "cell_dislocations_normal_distribution_width",
      0.0,
      "cell_dislocations_normal_distribution_width >= 0.0",
      "Width of the normal distribution to assign to the initial cell dislocation value. This is "
      "given as a fraction of the initial_cell_dislocation_density.");
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
      "wall_dislocations_normal_distribution_width",
      0.0,
      "wall_dislocations_normal_distribution_width >= 0.0",
      "Width of the normal distribution to assign to the initial wall dislocation value. This is "
      "given as a fraction of the initial_wall_dislocation_density.");
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
  params.addParam<FunctionName>(
      "effective_stress_forcing_function",
      "Optional forcing function for the effective stress. If provided, the effective stress will "
      "be reset to the function value at the beginning of the timestep. Used for testing purposes "
      "only.");

  params.addParam<unsigned int>("seed", 0, "Random number generator seed");
  params.addParam<std::string>("stress_unit", "Pa", "unit of stress");

  // use std::string here to avoid automatic absolute path expansion
  params.addParam<FileName>("model", "LaRomance model JSON datafile");
  params.addParam<FileName>("export_model", "Write LaRomance model to JSON datafile");

  params.addParamNamesToGroup(
      "cell_dislocation_density_forcing_function wall_dislocation_density_forcing_function "
      "old_creep_strain_forcing_function effective_stress_forcing_function seed stress_unit",
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
    _max_cell_increment(this->template getParam<Real>("max_relative_cell_dislocation_increment")),
    _cell_function(this->isParamValid("cell_dislocation_density_forcing_function")
                       ? &this->getFunction("cell_dislocation_density_forcing_function")
                       : NULL),
    _cell_dislocation_increment(0.0),
    _wall_dislocations(
        this->template declareGenericProperty<Real, is_ad>(this->_base_name + "wall_dislocations")),
    _wall_dislocations_old(
        this->template getMaterialPropertyOld<Real>(this->_base_name + "wall_dislocations")),
    _max_wall_increment(this->template getParam<Real>("max_relative_wall_dislocation_increment")),
    _wall_function(this->isParamValid("wall_dislocation_density_forcing_function")
                       ? &this->getFunction("wall_dislocation_density_forcing_function")
                       : NULL),
    _stress_function(this->isParamValid("effective_stress_forcing_function")
                         ? &this->getFunction("effective_stress_forcing_function")
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
    _second_partition_weight(
        this->template declareGenericProperty<Real, is_ad>("partition_weight")),

    _derivative(0.0),
    _old_input_values(3),
    _wall_dislocations_step(this->template declareGenericProperty<Real, is_ad>(
        this->_base_name + "wall_dislocations_step")),
    _cell_dislocations_step(this->template declareGenericProperty<Real, is_ad>(
        this->_base_name + "cell_dislocations_step")),
    _plastic_strain_increment(),
    _number_of_substeps(
        this->template declareProperty<Real>(this->_base_name + "number_of_substeps")),
    _index_name(_window_failure.size())
{
  this->_check_range = true; // this may not be necessary?

  _index_name[_cell_input_index] = "cell";
  _index_name[_wall_input_index] = "wall";
  _index_name[_stress_input_index] = "stress";
  _index_name[_old_strain_input_index] = "old strain";
  _index_name[_temperature_input_index] = "temperature";

  _window_failure[_cell_input_index].first =
      parameters.get<MooseEnum>("cell_input_window_low_failure").getEnum<WindowFailure>();
  _window_failure[_cell_input_index].second =
      parameters.get<MooseEnum>("cell_input_window_high_failure").getEnum<WindowFailure>();
  _window_failure[_wall_input_index].first =
      parameters.get<MooseEnum>("wall_input_window_low_failure").getEnum<WindowFailure>();
  _window_failure[_wall_input_index].second =
      parameters.get<MooseEnum>("wall_input_window_high_failure").getEnum<WindowFailure>();
  _window_failure[_stress_input_index].first =
      parameters.get<MooseEnum>("stress_input_window_low_failure").getEnum<WindowFailure>();
  _window_failure[_stress_input_index].second =
      parameters.get<MooseEnum>("stress_input_window_high_failure").getEnum<WindowFailure>();
  _window_failure[_old_strain_input_index].first =
      parameters.get<MooseEnum>("old_strain_input_window_low_failure").getEnum<WindowFailure>();
  _window_failure[_old_strain_input_index].second =
      parameters.get<MooseEnum>("old_strain_input_window_high_failure").getEnum<WindowFailure>();
  _window_failure[_temperature_input_index].first =
      parameters.get<MooseEnum>("temperature_input_window_low_failure").getEnum<WindowFailure>();
  _window_failure[_temperature_input_index].second =
      parameters.get<MooseEnum>("temperature_input_window_high_failure").getEnum<WindowFailure>();
  if (_environmental)
  {
    _index_name[_environmental_input_index] = "environmental";
    _window_failure[_environmental_input_index].first =
        parameters.get<MooseEnum>("environment_input_window_low_failure").getEnum<WindowFailure>();
    _window_failure[_environmental_input_index].second =
        parameters.get<MooseEnum>("environment_input_window_high_failure").getEnum<WindowFailure>();
  }

  // load JSON datafile
  if (this->isParamValid("model"))
  {
    const auto model_file_name = this->getDataFileName("model");
    std::ifstream model_file(model_file_name.c_str());
    model_file >> _json;
  }

  setupUnitConversionFactors(parameters);
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::exportJSON()
{
  _json["strain_cutoff"] = getStrainCutoff();
  _json["transform"] = getTransform();
  _json["transform_coefs"] = getTransformCoefs();
  _json["input_limits"] = getInputLimits();
  _json["normalization_limits"] = getNormalizationLimits();
  _json["coefs"] = getCoefs();
  _json["tiling"] = getTilings();
  _json["cutoff"] = getStrainCutoff();
}

template <bool is_ad>
bool
LAROMANCEStressUpdateBaseTempl<is_ad>::substeppingCapabilityEnabled()
{
  return this->_use_substepping != RadialReturnStressUpdateTempl<is_ad>::SubsteppingType::NONE;
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::setupUnitConversionFactors(
    const InputParameters & parameters)
{
  // Stress unit conversion factor
  const MooseUnits stress_unit_to("MPa");
  const MooseUnits stress_unit_from(parameters.get<std::string>("stress_unit"));
  _stress_ucf = stress_unit_to.convert(1, stress_unit_from);
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::initialSetup()
{
  // export models that are compiled in
  if (this->isParamValid("export_model"))
  {
    exportJSON();
    std::ofstream out(this->template getParam<FileName>("export_model").c_str());
    out << _json;
  }

  // Pull in relevant ROM information and do sanity checks
  _transform = getTransform();
  _transform_coefs = getTransformCoefs();
  _input_limits = getInputLimits();
  _normalization_limits = getNormalizationLimits();
  _coefs = getCoefs();
  _tiling = getTilings();
  _cutoff = getStrainCutoff();
  // resize containers to be filled later based on partition dimension
  // and immediately run some sanity checks
  _num_partitions = _transform.size();
  if (_num_partitions < 1 || _num_partitions > 2)
    mooseError(
        "In ", _name, ": First dimension of getTransform must be either size one or size two");
  if (_transform[0].size() < 1)
    mooseError("In ", _name, ": Transform is not the correct shape");
  _num_outputs = _transform[0][0].size();
  if (_num_outputs != 3)
    mooseError("In ",
               _name,
               ": ",
               _num_outputs,
               " outputs detected. Three and only three outputs are currently supported.");

  _num_inputs = _transform[0][0][0].size();
  if (_num_inputs != 5 && _num_inputs != 6)
    mooseError("In ",
               _name,
               ": ",
               _num_inputs,
               " inputs detected. Only five or six inputs currently supported.");
  if (_num_inputs == 6 && !_environmental)
    this->template paramError(
        "environmental_factor",
        "Number of ROM inputs indicate environmental factor is required to be coupled.");
  if (_num_inputs != 6 && _environmental)
    this->template paramError(
        "environmental_factor",
        "Number of ROM inputs indicate environmental factor is not implemented, but "
        "environmental factor coupled.");
  _num_tiles.resize(_num_partitions);
  _num_coefs.resize(_num_partitions);
  _degree.resize(_num_partitions);
  _precomputed_vals.resize(_num_partitions);
  _rom_inputs.resize(_num_partitions);
  _polynomial_inputs.resize(_num_partitions);
  _non_stress_weights.resize(_num_partitions);
  _weights.resize(_num_partitions);
  _partition_weights.resize(_num_partitions);
  _dpartition_weight_dstress.resize(_num_partitions);
  _transformed_normalization_limits.resize(_num_partitions);
  _makeframe_helper.resize(_num_partitions);
  _global_limits.resize(_num_inputs);
  // temporarily fill global limits with extreme numerical values, to later update
  for (unsigned int i = 0; i < _num_inputs; ++i)
    _global_limits[i] = {std::numeric_limits<Real>::max(), 0.0};

  // start loop over partitions to perform sanity checks, set global limits,
  // and print global configurations
  if (_transform.size() != _num_partitions || _transform_coefs.size() != _num_partitions ||
      _input_limits.size() != _num_partitions || _normalization_limits.size() != _num_partitions ||
      _coefs.size() != _num_partitions || _tiling.size() != _num_partitions ||
      _cutoff.size() != _num_partitions)
    mooseError(
        "In ", _name, ": one of the ROM inputs does not have the correct number of partitions");

  for (unsigned int p = 0; p < _num_partitions; ++p)
  {
    _num_tiles[p] = _transform[p].size();
    if (!_num_tiles[p])
      mooseError("In ", _name, ": No tiles detected. Double check your ROM input");

    bool correct_shape = true;
    if (_transform[p].size() != _num_tiles[p] || _transform_coefs[p].size() != _num_tiles[p] ||
        _input_limits[p].size() != _num_tiles[p] ||
        _normalization_limits[p].size() != _num_tiles[p] || _coefs[p].size() != _num_tiles[p])
      correct_shape = false;
    if (_tiling[p].size() != _num_inputs)
      correct_shape = false;
    if (_coefs[p][0].size() == 0)
      correct_shape = false;
    _num_coefs[p] = _coefs[p][0][0].size();

    // start loop over tiles to perform sanity checks.
    for (unsigned int t = 0; t < _num_tiles[p]; ++t)
    {
      if (_transform[p][t].size() != _num_outputs ||
          _transform_coefs[p][t].size() != _num_outputs || _coefs[p][t].size() != _num_outputs)
        correct_shape = false;
      for (unsigned int o = 0; o < _num_outputs; ++o)
        if (_transform[p][t][o].size() != _num_inputs ||
            _transform_coefs[p][t][o].size() != _num_inputs ||
            _coefs[p][t][o].size() != _num_coefs[p])
          correct_shape = false;
      if (_input_limits[p][t].size() != _num_inputs ||
          _normalization_limits[p][t].size() != _num_inputs)
        correct_shape = false;
      for (unsigned int i = 0; i < _num_inputs; ++i)
        if (_input_limits[p][t][i].size() != 2 || _normalization_limits[p][t][i].size() != 2)
          correct_shape = false;
    }

    if (!correct_shape)
      mooseError("In ", _name, ": ROM data is not the right shape.");

    _degree[p] = std::pow(_num_coefs[p], 1.0 / _num_inputs);
    if (!_degree[p] || _degree[p] > 4)
      mooseError("In ", _name, ": degree must be 1, 2, 3 or 4.");

    // Check input limits and find global limits of all partitions. Note that this will return the
    // extremes of the model! If the model is not flat along one input limit, then global limits
    // will not truely capture the mutli-dimensionality of the problem. Consequently, the user may
    // find input values that result in errors in the partition weight computation.
    for (unsigned int i = 0; i < _num_inputs; ++i)
    {
      for (unsigned int t = 0; t < _num_tiles[p]; ++t)
      {
        if (_input_limits[p][t][i][0] >= _input_limits[p][t][i][1])
          mooseError("In ", _name, ": Input limits are ordered incorrectly");
        _global_limits[i].first = std::min(_global_limits[i].first, _input_limits[p][t][i][0]);
        _global_limits[i].second = std::max(_global_limits[i].second, _input_limits[p][t][i][1]);
      }
    }

    // Precompute helper containers
    _transformed_normalization_limits[p] = getTransformedLimits(p, _normalization_limits[p]);
    _makeframe_helper[p] = getMakeFrameHelper(p);

    // Prepare containers for each partition
    _precomputed_vals[p].resize(_num_tiles[p], std::vector<GenericReal<is_ad>>(_num_coefs[p]));
    _rom_inputs[p].resize(_num_tiles[p], std::vector<GenericReal<is_ad>>(_num_inputs));
    _polynomial_inputs[p].resize(_num_tiles[p],
                                 std::vector<std::vector<GenericReal<is_ad>>>(
                                     _num_inputs, std::vector<GenericReal<is_ad>>(_degree[p])));
    _non_stress_weights[p].resize(_num_tiles[p]);
    _weights[p].resize(_num_tiles[p], 0);
  }
  // Prepare containers independent of partition
  _input_values.resize(_num_inputs);

  if (_verbose)
  {
    Moose::err << "ROM model info: " << _name << "\n";
    Moose::err << " number of tiles, partition 1: " << _num_tiles[0] << "\n";
    if (_num_partitions > 1)
      Moose::err << " number of tiles, partition 2: " << _num_tiles[1] << "\n";
    Moose::err << " number of outputs: " << _num_outputs << "\n";
    Moose::err << " number of inputs: " << _num_inputs << "\n";
    Moose::err << " degree (max Legendre degree + constant), partition 1: " << _degree[0] << "\n";
    if (_num_partitions > 1)
      Moose::err << " degree (max Legendre degree + constant), partition 2: " << _degree[1] << "\n";
    Moose::err << " number of coefficients, partition 1: " << _num_coefs[0] << "\n";
    if (_num_partitions > 1)
      Moose::err << " number of coefficients, partition 2: " << _num_coefs[1] << "\n";
    Moose::err << " Global limits:\n  cell dislocations ("
               << _global_limits[_cell_input_index].first << " - "
               << _global_limits[_cell_input_index].second << ")\n";
    Moose::err << "  wall dislocations (" << _global_limits[_wall_input_index].first << " - "
               << _global_limits[_wall_input_index].second << ")\n";
    Moose::err << "  Stress (" << _global_limits[_stress_input_index].first << " - "
               << _global_limits[_stress_input_index].second << ")\n";
    Moose::err << "  Old strain (" << _global_limits[_old_strain_input_index].first << " - "
               << _global_limits[_old_strain_input_index].second << ")\n";
    Moose::err << "  Temperature (" << _global_limits[_temperature_input_index].first << " - "
               << _global_limits[_temperature_input_index].second << ")\n";
    if (_environmental)
      Moose::err << "  Environmental factor (" << _global_limits[_environmental_input_index].first
                 << " - " << _global_limits[_environmental_input_index].second << ")\n";
    Moose::err << std::endl;
  }
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::initQpStatefulProperties()
{
  MooseRandom rng;
  rng.seed(0, this->template getParam<unsigned int>("seed"));

  _cell_dislocations[_qp] = rng.randNormal(
      this->template getParam<Real>("initial_cell_dislocation_density"),
      this->template getParam<Real>("initial_cell_dislocation_density") *
          this->template getParam<Real>("cell_dislocations_normal_distribution_width"));
  _wall_dislocations[_qp] = rng.randNormal(
      this->template getParam<Real>("initial_wall_dislocation_density"),
      this->template getParam<Real>("initial_wall_dislocation_density") *
          this->template getParam<Real>("wall_dislocations_normal_distribution_width"));

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
LAROMANCEStressUpdateBaseTempl<is_ad>::resetIncrementalMaterialProperties()
{
  _cell_dislocation_increment = 0.0;
  _wall_dislocation_increment = 0.0;

  _plastic_strain_increment.zero();

  _wall_dislocations_step[_qp] = 0.0;
  _cell_dislocations_step[_qp] = 0.0;
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::storeIncrementalMaterialProperties(
    const unsigned int total_number_of_substeps)
{
  _wall_dislocations_step[_qp] += _wall_dislocation_increment;
  _cell_dislocations_step[_qp] += _cell_dislocation_increment;
  _number_of_substeps[_qp] = total_number_of_substeps;
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::computeStressInitialize(
    const GenericReal<is_ad> & effective_trial_stress,
    const GenericRankFourTensor<is_ad> & elasticity_tensor)
{
  RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeStressInitialize(effective_trial_stress,
                                                                         elasticity_tensor);
  // Previous substep creep strain
  RankTwoTensor creep_strain_substep = this->_creep_strain_old[_qp] + _plastic_strain_increment;

  // Prepare old values
  _old_input_values[_cell_output_index] =
      _cell_function
          ? _cell_function->value(_t, _q_point[_qp])
          : (_cell_dislocations_old[_qp] + MetaPhysicL::raw_value(_cell_dislocations_step[_qp]));
  _old_input_values[_wall_output_index] =
      _wall_function
          ? _wall_function->value(_t, _q_point[_qp])
          : (_wall_dislocations_old[_qp] + MetaPhysicL::raw_value(_wall_dislocations_step[_qp]));
  _old_input_values[_strain_output_index] =
      _creep_strain_old_forcing_function
          ? _creep_strain_old_forcing_function->value(_t, _q_point[_qp])
          : MetaPhysicL::raw_value(
                std::sqrt((creep_strain_substep).doubleContraction(creep_strain_substep) / 1.5));

  // Prepare input
  _input_values[_cell_input_index] = _old_input_values[_cell_output_index];
  _input_values[_wall_input_index] = _old_input_values[_wall_output_index];
  _input_values[_stress_input_index] = _stress_function ? _stress_function->value(_t, _q_point[_qp])
                                                        : effective_trial_stress * 1.0e-6;
  _input_values[_old_strain_input_index] = _old_input_values[_strain_output_index];
  _input_values[_temperature_input_index] = _temperature[_qp];
  if (_environmental)
    _input_values[_environmental_input_index] = (*_environmental)[_qp];

  // Determine tile weight and check if input is in range
  for (unsigned int p = 0; p < _num_partitions; ++p)
    std::fill(_non_stress_weights[p].begin(), _non_stress_weights[p].end(), 1.0);
  for (unsigned int i = 0; i < _num_inputs; i++)
    if (i != _stress_input_index)
      computeTileWeight(_non_stress_weights, _input_values[i], i);

  // Precompute transformed input and prebuild polynomials for inputs other than strain
  precomputeROM(_strain_output_index);
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::computeTileWeight(
    std::vector<std::vector<GenericReal<is_ad>>> & weights,
    GenericReal<is_ad> & input,
    const unsigned int in_index,
    const bool derivative)
{
  mooseAssert(std::isfinite(MetaPhysicL::raw_value(input)),
              "computeTileWeight input must be finite");

  for (unsigned int p = 0; p < _num_partitions; ++p)
  {
    for (unsigned int t = 0; t < _num_tiles[p]; ++t)
    {
      // If input is within a specfic tile's window of applicability
      if (input >= _input_limits[p][t][in_index][0] && input <= _input_limits[p][t][in_index][1])
      {
        // If there is not more than one tile for this input, set derivative of
        // weight to zero
        if (_tiling[p][in_index] == 1)
        {
          if (derivative)
            weights[p][t] = 0.0;
        }
        else
        {
          // Flag to ensure weights are applied only once
          bool overlap = false;
          for (unsigned int tt = 0; tt < _num_tiles[p]; ++tt)
          {
            if (!overlap && t != tt)
            {
              // If input is within another tile's window of applicability, check to see if inputs
              // place us in that tile and ensure the two tiles are different in the dimension of
              // interest
              if (areTilesNotIdentical(p, t, tt, in_index) && checkInTile(p, tt))
              {
                overlap = true;

                // If current tile is below the second tile's window of applicability
                if (_input_limits[p][t][in_index][0] < _input_limits[p][tt][in_index][0] &&
                    _input_limits[p][t][in_index][1] > _input_limits[p][tt][in_index][0])
                {
                  weights[p][t] *= sigmoid(_input_limits[p][tt][in_index][0],
                                           _input_limits[p][t][in_index][1],
                                           input,
                                           derivative);
                }
                // If current tile is above the second tile's window of applicability
                else if (_input_limits[p][t][in_index][0] > _input_limits[p][tt][in_index][0] &&
                         _input_limits[p][t][in_index][0] < _input_limits[p][tt][in_index][1])
                {
                  if (derivative)
                    weights[p][t] *= -sigmoid(_input_limits[p][t][in_index][0],
                                              _input_limits[p][tt][in_index][1],
                                              input,
                                              derivative);
                  else
                    weights[p][t] *= (1.0 - sigmoid(_input_limits[p][t][in_index][0],
                                                    _input_limits[p][tt][in_index][1],
                                                    input));
                }
              }
            }
          }

          // If not overlapping, weight is not updated, and the derivative of tile weight is set to
          // zero
          if (!overlap && derivative)
            weights[p][t] = 0.0;
        }
      }
      // If input is below the lower tile limit
      else if (input < _input_limits[p][t][in_index][0])
      {
        // If the lower tile limit equals the lower global limit
        if (_input_limits[p][t][in_index][0] == _global_limits[in_index].first)
        {
          if (_window_failure[in_index].first == WindowFailure::EXTRAPOLATE)
          {
            if (derivative)
              weights[p][t] *= -sigmoid(0.0, _input_limits[p][t][in_index][0], input, derivative);
            else
              weights[p][t] *= (1.0 - sigmoid(0.0, _input_limits[p][t][in_index][0], input));
            input = _input_limits[p][t][in_index][0];
          }
          else if (_window_failure[in_index].first == WindowFailure::USELIMIT)
            input = _input_limits[p][t][in_index][0];
          else
          {
            weights[p][t] = 0.0;
            std::stringstream msg;
            msg << "In " << _name << ": " << _index_name[in_index]
                << " input parameter with value (" << MetaPhysicL::raw_value(input)
                << ") is out of lower global range (" << _input_limits[p][t][in_index][0] << ")";

            // Set input to edge of limit so it is found later in computePartitionWeights
            input = _input_limits[p][t][in_index][0];

            if (_window_failure[in_index].first == WindowFailure::WARN)
              mooseWarning(msg.str());
            else if (_window_failure[in_index].first == WindowFailure::ERROR)
              mooseError(msg.str());
            else if (_window_failure[in_index].first == WindowFailure::EXCEPTION)
              mooseException(msg.str());
            // if (_window_failure[in_index].first == WindowFailure::DONOTHING) <- nothing is done
          }
        }
        // if input below tile limit, update weight of tile to be zero
        else
          weights[p][t] = 0.0;
      }
      // If input is above the upper tile limit
      else if (input > _input_limits[p][t][in_index][1])
      {
        if (_input_limits[p][t][in_index][1] == _global_limits[in_index].second)
        {
          if (_window_failure[in_index].second == WindowFailure::EXTRAPOLATE)
            mooseError("Internal error. Extrapolate not appropriate for upper bound");
          else if (_window_failure[in_index].second == WindowFailure::USELIMIT)
            input = _input_limits[p][t][in_index][1];
          else
          {
            weights[p][t] = 0.0;
            std::stringstream msg;
            msg << "In " << _name << ": " << _index_name[in_index]
                << " input parameter with value (" << MetaPhysicL::raw_value(input)
                << ") is out of upper global range (" << _input_limits[p][t][in_index][1] << ")";

            // Set input to edge of limit so it is found later in computePartitionWeights
            input = _input_limits[p][t][in_index][1];

            if (_window_failure[in_index].second == WindowFailure::WARN)
              mooseWarning(msg.str());
            else if (_window_failure[in_index].second == WindowFailure::ERROR)
              mooseError(msg.str());
            else if (_window_failure[in_index].second == WindowFailure::EXCEPTION)
              mooseException(msg.str());
            // if (_window_failure[in_index].second == WindowFailure::DONOTHING) <- nothing is done
          }
        }
        // if input above tile limit, update weight of tile to be zero
        else
          weights[p][t] = 0.0;
      }
      // If input is outside window of applicability, weight is zero
      else
        mooseError("In ", _name, ": Internal error. Outside input limits, input=", input);
    }
  }
}

template <bool is_ad>
bool
LAROMANCEStressUpdateBaseTempl<is_ad>::checkInTile(const unsigned int p, const unsigned int t) const
{
  for (unsigned int i = 0; i < _num_inputs; ++i)
    if (_input_values[i] < _input_limits[p][t][i][0] ||
        _input_values[i] > _input_limits[p][t][i][1])
      return false;
  return true;
}

template <bool is_ad>
bool
LAROMANCEStressUpdateBaseTempl<is_ad>::areTilesNotIdentical(const unsigned int p,
                                                            const unsigned int t,
                                                            const unsigned int tt,
                                                            const unsigned int in_index)
{
  if (_input_limits[p][t][in_index][0] != _input_limits[p][tt][in_index][0] &&
      _input_limits[p][t][in_index][1] != _input_limits[p][tt][in_index][1])
    return true;
  else
    return false;
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEStressUpdateBaseTempl<is_ad>::computeResidual(
    const GenericReal<is_ad> & effective_trial_stress, const GenericReal<is_ad> & scalar)
{
  mooseAssert(std::isfinite(MetaPhysicL::raw_value(effective_trial_stress)),
              "computeResidual: effective_trial_stress must be finite");
  mooseAssert(std::isfinite(MetaPhysicL::raw_value(scalar)),
              "computeResidual: scalar must be finite!");
  // Update new stress
  GenericReal<is_ad> trial_stress_mpa = _stress_function
                                            ? _stress_function->value(_t, _q_point[_qp])
                                            : effective_trial_stress * _stress_ucf;
  GenericReal<is_ad> dtrial_stress_dscalar = 0.0;

  // Update stress if strain is being applied, i.e. non-testing simulation
  if (this->_apply_strain)
  {
    trial_stress_mpa -= this->_three_shear_modulus * scalar * _stress_ucf;
    dtrial_stress_dscalar -= this->_three_shear_modulus * _stress_ucf;
  }
  _input_values[_stress_input_index] = trial_stress_mpa;

  // Update weights for each partition with new stress
  for (unsigned int p = 0; p < _num_partitions; ++p)
    _weights[p] = _non_stress_weights[p];
  computeTileWeight(_weights, _input_values[_stress_input_index], _stress_input_index);
  auto dweights_dstress = _non_stress_weights;
  computeTileWeight(
      dweights_dstress, _input_values[_stress_input_index], _stress_input_index, true);

  computePartitionWeights(_partition_weights, _dpartition_weight_dstress);

  // Save extrapolation as a material property in order quantify adequate tiling range
  _extrapolation[_qp] = 0.0;
  for (unsigned int p = 0; p < _num_partitions; ++p)
    for (unsigned int t = 0; t < _num_tiles[p]; ++t)
      _extrapolation[_qp] += MetaPhysicL::raw_value(_weights[p][t] * _partition_weights[p]);

  GenericReal<is_ad> total_rom_effective_strain_inc = 0.0;
  GenericReal<is_ad> dtotal_rom_effective_strain_inc_dstress = 0.0;

  // Run ROM if all values are within windows.
  for (unsigned int p = 0; p < _num_partitions; p++)
  {
    if (_partition_weights[p])
    {
      // compute weight normalizing factor
      GenericReal<is_ad> weight_normalizer = 0;
      unsigned int number_of_active_tiles = 0;
      for (unsigned int t = 0; t < _num_tiles[p]; ++t)
      {
        // count number of active tiles
        number_of_active_tiles += checkInTile(p, t);
        if (_weights[p][t])
        {
          // tile normalization factor (sum of tile weights)
          weight_normalizer += _weights[p][t];
        }
      }

      // normalize weights only when 3 tiles overlap
      if (number_of_active_tiles == 3)
        for (unsigned int t = 0; t < _num_tiles[p]; ++t)
        {
          _weights[p][t] /= weight_normalizer;
          dweights_dstress[p][t] /= weight_normalizer;
        }

      for (unsigned int t = 0; t < _num_tiles[p]; ++t)
        if (_weights[p][t])
        {
          const GenericReal<is_ad> rom = computeROM(t, p, _strain_output_index);
          if (rom == std::numeric_limits<float>::infinity())
            mooseError("In ", _name, ": Output for strain increment reaches infinity: ", rom);

          total_rom_effective_strain_inc += _partition_weights[p] * _weights[p][t] * rom;

          dtotal_rom_effective_strain_inc_dstress +=
              _partition_weights[p] * _weights[p][t] * computeROM(t, p, _strain_output_index, true);
          if (_dpartition_weight_dstress[p])
            dtotal_rom_effective_strain_inc_dstress +=
                _dpartition_weight_dstress[p] * _weights[p][t] * rom;
          if (dweights_dstress[p][t])
            dtotal_rom_effective_strain_inc_dstress +=
                _partition_weights[p] * dweights_dstress[p][t] * rom;
        }
    }
  }

  if (_verbose)
  {
    Moose::err << std::setprecision(9);
    GenericReal<is_ad> environmental = 0.0;
    if (_environmental)
      environmental = (*_environmental)[_qp];
    Moose::err << "Verbose information from " << _name << ": \n";
    Moose::err << " dt: " << _dt << "\n";
    Moose::err << " old cell disl: " << _old_input_values[_cell_output_index] << "\n";
    Moose::err << " old wall disl: " << _old_input_values[_wall_output_index] << "\n";
    Moose::err << " initial stress (MPa): "
               << MetaPhysicL::raw_value(effective_trial_stress) * _stress_ucf << "\n";
    Moose::err << " temperature: " << MetaPhysicL::raw_value(_temperature[_qp]) << "\n";
    Moose::err << " environmental factor: " << MetaPhysicL::raw_value(environmental) << "\n";
    Moose::err << " calculated scalar strain value: " << MetaPhysicL::raw_value(scalar) << "\n";
    Moose::err << " trial stress into rom (MPa): " << MetaPhysicL::raw_value(trial_stress_mpa)
               << "\n";
    Moose::err << " old effective strain: " << _old_input_values[_strain_output_index] << "\n";
    Moose::err << " extrapolation: " << MetaPhysicL::raw_value(_extrapolation[_qp]) << "\n";
    Moose::err << " partition 2 weight: " << MetaPhysicL::raw_value(_second_partition_weight[_qp])
               << "\n";
    Moose::err << " weights by tile, partition 1: ";
    for (unsigned int t = 0; t < _num_tiles[0]; ++t)
      Moose::err << " (" << t << ", " << MetaPhysicL::raw_value(_weights[0][t]) << ") ";
    Moose::err << "\n";
    if (_num_partitions > 1)
    {
      Moose::err << " weights by tile, partition 2: ";
      for (unsigned int t = 0; t < _num_tiles[1]; ++t)
        Moose::err << " (" << t << ", " << MetaPhysicL::raw_value(_weights[1][t]) << ") ";
    }
    Moose::err << "\n";
    Moose::err << " nonstress weights by tile, partition 1: ";
    for (unsigned int t = 0; t < _num_tiles[0]; ++t)
      Moose::err << " (" << t << ", " << MetaPhysicL::raw_value(_non_stress_weights[0][t]) << ") ";
    Moose::err << "\n";
    if (_num_partitions > 1)
    {
      Moose::err << " nonstress weights by tile, partition 2: ";
      for (unsigned int t = 0; t < _num_tiles[1]; ++t)
        Moose::err << " (" << t << ", " << MetaPhysicL::raw_value(_non_stress_weights[1][t])
                   << ") ";
    }
    Moose::err << "\n";
    Moose::err << " effective strain increment: "
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
  for (unsigned int p = 0; p < _num_partitions; ++p)
  {
    for (unsigned int t = 0; t < _num_tiles[p]; ++t)
    {
      // Only precompute for tiles that don't have zero weight
      if (_non_stress_weights[p][t])
      {
        for (unsigned int i = 0; i < _num_inputs; ++i)
        {
          if (i != _stress_input_index)
          {
            _rom_inputs[p][t][i] =
                normalizeInput(_input_values[i],
                               _transform[p][t][out_index][i],
                               _transform_coefs[p][t][out_index][i],
                               _transformed_normalization_limits[p][t][out_index][i]);
            buildPolynomials(p, _rom_inputs[p][t][i], _polynomial_inputs[p][t][i]);
          }
        }
        precomputeValues(
            p, _coefs[p][t][out_index], _polynomial_inputs[p][t], _precomputed_vals[p][t]);
      }
    }
  }
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEStressUpdateBaseTempl<is_ad>::computeROM(const unsigned int t,
                                                  const unsigned int p,
                                                  const unsigned out_index,
                                                  const bool derivative)
{
  // Update due to new stress
  _rom_inputs[p][t][_stress_input_index] =
      normalizeInput(_input_values[_stress_input_index],
                     _transform[p][t][out_index][_stress_input_index],
                     _transform_coefs[p][t][out_index][_stress_input_index],
                     _transformed_normalization_limits[p][t][out_index][_stress_input_index]);

  buildPolynomials(
      p, _rom_inputs[p][t][_stress_input_index], _polynomial_inputs[p][t][_stress_input_index]);

  // Compute ROM values
  const GenericReal<is_ad> rom_output =
      computeValues(p, _precomputed_vals[p][t], _polynomial_inputs[p][t]);

  // Return converted output if not derivative
  if (!derivative)
    return convertOutput(p, _old_input_values, rom_output, out_index);

  const GenericReal<is_ad> drom_input =
      normalizeInput(_input_values[_stress_input_index],
                     _transform[p][t][out_index][_stress_input_index],
                     _transform_coefs[p][t][out_index][_stress_input_index],
                     _transformed_normalization_limits[p][t][out_index][_stress_input_index],
                     derivative);

  std::vector<GenericReal<is_ad>> dpolynomial_inputs(_degree[p], 0.0);
  buildPolynomials(
      p, _rom_inputs[p][t][_stress_input_index], dpolynomial_inputs, drom_input, derivative);

  const GenericReal<is_ad> drom_output = computeValues(
      p, _precomputed_vals[p][t], _polynomial_inputs[p][t], dpolynomial_inputs, derivative);
  return convertOutput(p, _old_input_values, rom_output, out_index, drom_output, derivative);
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
    const unsigned int p,
    const GenericReal<is_ad> & rom_input,
    std::vector<GenericReal<is_ad>> & polynomial_inputs,
    const GenericReal<is_ad> & drom_input,
    const bool derivative)
{
  for (unsigned int d = 0; d < _degree[p]; ++d)
  {
    polynomial_inputs[d] = computePolynomial(rom_input, d);

    if (derivative)
      polynomial_inputs[d] = drom_input * computePolynomial(rom_input, d, derivative);
  }
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::precomputeValues(
    const unsigned int p,
    const std::vector<Real> & coefs,
    const std::vector<std::vector<GenericReal<is_ad>>> & polynomial_inputs,
    std::vector<GenericReal<is_ad>> & precomputed)
{
  for (unsigned int c = 0; c < _num_coefs[p]; ++c)
  {
    precomputed[c] = coefs[c];
    for (unsigned int i = 0; i < _num_inputs; ++i)
      if (i != _stress_input_index)
        precomputed[c] *= polynomial_inputs[i][_makeframe_helper[p][c + _num_coefs[p] * i]];
  }
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEStressUpdateBaseTempl<is_ad>::computeValues(
    const unsigned int p,
    const std::vector<GenericReal<is_ad>> & precomputed,
    const std::vector<std::vector<GenericReal<is_ad>>> & polynomial_inputs,
    const std::vector<GenericReal<is_ad>> & dpolynomial_inputs,
    const bool derivative)
{
  GenericReal<is_ad> rom_output = 0.0;
  for (unsigned int c = 0; c < _num_coefs[p]; ++c)
  {
    if (!derivative)
      rom_output +=
          precomputed[c] *
          polynomial_inputs[_stress_input_index]
                           [_makeframe_helper[p][c + _num_coefs[p] * _stress_input_index]];

    else
      rom_output +=
          precomputed[c] *
          dpolynomial_inputs[_makeframe_helper[p][c + _num_coefs[p] * _stress_input_index]];
  }
  return rom_output;
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEStressUpdateBaseTempl<is_ad>::convertOutput(const unsigned int p,
                                                     const std::vector<Real> & old_input_values,
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
  mooseAssert(expout > 0.0, "ROM calculated strain increment must be positive");

  if (expout > _cutoff[p])
    expout -= _cutoff[p];
  else
    expout = -_cutoff[p] * _cutoff[p] / expout + _cutoff[p];

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
    const unsigned int p, const std::vector<std::vector<std::vector<Real>>> limits)
{
  std::vector<std::vector<std::vector<std::vector<Real>>>> transformed_limits(
      _num_tiles[p],
      std::vector<std::vector<std::vector<Real>>>(
          _num_outputs, std::vector<std::vector<Real>>(_num_inputs, std::vector<Real>(3))));
  for (unsigned int t = 0; t < _num_tiles[p]; ++t)
  {
    for (unsigned int o = 0; o < _num_outputs; ++o)
    {
      for (unsigned int i = 0; i < _num_inputs; ++i)
      {
        for (unsigned int k = 0; k < 2; ++k)
        {
          transformed_limits[t][o][i][k] = limits[t][i][k];
          convertValue(
              transformed_limits[t][o][i][k], _transform[p][t][o][i], _transform_coefs[p][t][o][i]);
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
LAROMANCEStressUpdateBaseTempl<is_ad>::getMakeFrameHelper(const unsigned int p) const
{
  std::vector<unsigned int> v(_num_coefs[p] * _num_inputs);

  for (unsigned int numcoeffs = 0; numcoeffs < _num_coefs[p]; ++numcoeffs)
    for (unsigned int invar = 0; invar < _num_inputs; ++invar)
      v[numcoeffs + _num_coefs[p] * invar] =
          numcoeffs / MathUtils::pow(_degree[p], invar) % _degree[p];

  return v;
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::computeStressFinalize(
    const GenericRankTwoTensor<is_ad> & plastic_strain_increment)
{
  _cell_dislocation_increment = 0.0;
  _wall_dislocation_increment = 0.0;
  if (_input_values[_stress_input_index])
  {
    precomputeROM(_cell_output_index);
    for (unsigned int p = 0; p < _num_partitions; ++p)
      if (_partition_weights[p])
        for (unsigned int t = 0; t < _num_tiles[p]; ++t)
          if (_weights[p][t])
            _cell_dislocation_increment +=
                _partition_weights[p] * _weights[p][t] * computeROM(t, p, _cell_output_index);
    precomputeROM(_wall_output_index);
    for (unsigned int p = 0; p < _num_partitions; ++p)
      if (_partition_weights[p])
        for (unsigned int t = 0; t < _num_tiles[p]; ++t)
          if (_weights[p][t])
            _wall_dislocation_increment +=
                _partition_weights[p] * _weights[p][t] * computeROM(t, p, _wall_output_index);
  }

  _cell_rate[_qp] = _cell_dislocation_increment / _dt;
  _wall_rate[_qp] = _wall_dislocation_increment / _dt;
  _cell_dislocations[_qp] = _old_input_values[_cell_output_index] + _cell_dislocation_increment;
  _wall_dislocations[_qp] = _old_input_values[_wall_output_index] + _wall_dislocation_increment;

  // For (possibly) substepping.
  _plastic_strain_increment += MetaPhysicL::raw_value(plastic_strain_increment);

  // Prevent the ROM from calculating and proceeding with negative dislocations
  if (_apply_strain && (_cell_dislocations[_qp] < 0.0 || _wall_dislocations[_qp] < 0.0))
    mooseException("In ",
                   _name,
                   ": Negative disclocation density calculated for cell (old : ",
                   MetaPhysicL::raw_value(_old_input_values[_cell_output_index]),
                   " increment: ",
                   MetaPhysicL::raw_value(_cell_dislocation_increment),
                   " value: ",
                   MetaPhysicL::raw_value(_cell_dislocations[_qp]),
                   ") or wall (old : ",
                   MetaPhysicL::raw_value(_old_input_values[_wall_output_index]),
                   " increment: ",
                   MetaPhysicL::raw_value(_wall_dislocation_increment),
                   " value: ",
                   MetaPhysicL::raw_value(_wall_dislocations[_qp]),
                   ").");

  if (_verbose)
  {
    Moose::err << " Finalized ROM output\n";
    Moose::err << "  effective creep strain increment: "
               << std::sqrt(2.0 / 3.0 *
                            MetaPhysicL::raw_value(_plastic_strain_increment.doubleContraction(
                                _plastic_strain_increment)))
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

  RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeStressFinalize(
      MetaPhysicL::raw_value(_plastic_strain_increment));
}

template <bool is_ad>
Real
LAROMANCEStressUpdateBaseTempl<is_ad>::computeTimeStepLimit()
{
  Real limited_dt = RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeTimeStepLimit();

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
  mooseAssert(std::isfinite(MetaPhysicL::raw_value(val)), "Sigmoid value should must be infinite");
  mooseAssert(MetaPhysicL::raw_value(val) >= lower, "Input value must be greater than lower limit");
  mooseAssert(MetaPhysicL::raw_value(val) <= upper, "Input value must be greater than upper limit");

  // normalize val between 0 and 1, then shift between -1 and 1
  const GenericReal<is_ad> x = 2.0 * (val - lower) / (upper - lower) - 1.0;

  if (x == -1.0)
  {
    if (derivative)
      return 0.0;
    else
      return 1.0;
  }
  else if (x == 1.0)
  {
    if (derivative)
      return 0.0;
    else
      return 0.0;
  }
  else if (x < 1.0 && x > -1.0)
  {
    const GenericReal<is_ad> plus = std::exp(-2.0 / (1.0 + x));
    const GenericReal<is_ad> minus = std::exp(-2.0 / (1.0 - x));

    if (!derivative)
      return 1.0 - plus / (plus + minus);

    const GenericReal<is_ad> dplus_dx = plus * 2.0 / Utility::pow<2>(1.0 + x);
    const GenericReal<is_ad> dminus_dx = -minus * 2.0 / Utility::pow<2>(1.0 - x);

    return (plus * dminus_dx - dplus_dx * minus) / Utility::pow<2>(plus + minus) * 2.0 /
           (upper - lower);
  }
  else
    mooseError("Internal error: Sigmoid, value: x is out of bounds. val=",
               val,
               " low=",
               lower,
               " high=",
               upper);
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::outputIterationSummary(std::stringstream * iter_output,
                                                              const unsigned int total_it)
{
  if (iter_output)
  {
    *iter_output << "At element " << this->_current_elem->id() << " _qp=" << _qp << " Coordinates "
                 << _q_point[_qp] << " block=" << this->_current_elem->subdomain_id() << '\n';
    *iter_output << " dt " << _dt << " old cell disl: " << _old_input_values[_cell_output_index]
                 << " old wall disl: " << _old_input_values[_wall_output_index]
                 << " old effective strain: " << _old_input_values[_strain_output_index] << "\n";

    *iter_output << " temp: " << MetaPhysicL::raw_value(_temperature[_qp]) << " environmental: "
                 << (_environmental ? MetaPhysicL::raw_value((*_environmental)[_qp]) : 0.0)
                 << " trial stress into rom (MPa): "
                 << MetaPhysicL::raw_value(_input_values[_stress_input_index])
                 << " cell: " << MetaPhysicL::raw_value(_input_values[_cell_input_index])
                 << " wall: " << MetaPhysicL::raw_value(_input_values[_wall_input_index])
                 << " old strain: "
                 << MetaPhysicL::raw_value(_input_values[_old_strain_input_index]) << "\n";
    *iter_output << "  partition 2 weight: "
                 << MetaPhysicL::raw_value(_second_partition_weight[_qp]) << "\n";
    *iter_output << "  weights by tile, partition 1: ";
    for (unsigned int t = 0; t < _num_tiles[0]; ++t)
      *iter_output << " (" << t << ", " << MetaPhysicL::raw_value(_weights[0][t]) << ") ";
    *iter_output << "\n";
    *iter_output << "  weights by tile, partition 2: ";
    for (unsigned int t = 0; t < _num_tiles[1]; ++t)
      *iter_output << " (" << t << ", " << MetaPhysicL::raw_value(_weights[1][t]) << ") ";
    *iter_output << "\n";
    *iter_output << "  nonstress weights by tile, partition 1: ";
    for (unsigned int t = 0; t < _num_tiles[0]; ++t)
      *iter_output << " (" << t << ", " << MetaPhysicL::raw_value(_non_stress_weights[0][t])
                   << ") ";
    *iter_output << "\n";
    *iter_output << "  nonstress weights by tile, partition 2: ";
    for (unsigned int t = 0; t < _num_tiles[1]; ++t)
      *iter_output << " (" << t << ", " << MetaPhysicL::raw_value(_non_stress_weights[1][t])
                   << ") ";
    *iter_output << "\n";
  }

  SingleVariableReturnMappingSolutionTempl<is_ad>::outputIterationSummary(iter_output, total_it);
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::outputIterationStep(
    std::stringstream * iter_output,
    const GenericReal<is_ad> & effective_trial_stress,
    const GenericReal<is_ad> & scalar,
    const Real reference_residual)
{
  SingleVariableReturnMappingSolutionTempl<is_ad>::outputIterationStep(
      iter_output, effective_trial_stress, scalar, reference_residual);
  if (iter_output)
    *iter_output << " derivative: "
                 << MetaPhysicL::raw_value(computeDerivative(effective_trial_stress, scalar))
                 << std::endl;
}

template <bool is_ad>
void
LAROMANCEStressUpdateBaseTempl<is_ad>::checkJSONKey(const std::string & key)
{
  if (!this->isParamValid("model"))
    this->paramError("model", "Specify a JSON data filename.");

  const auto model_file_name = this->_pars.rawParamVal("model");
  if (_json.empty())
    this->paramError("model", "The specified JSON data file '", model_file_name, "' is empty.");
  if (!_json.contains(key))
    this->paramError(
        "model", "The key '", key, "' is missing from the JSON data file '", model_file_name, "'.");
}

template class LAROMANCEStressUpdateBaseTempl<false>;
template class LAROMANCEStressUpdateBaseTempl<true>;
