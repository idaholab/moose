//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrystalPlasticityHCPDislocationSlipBeyerleinUpdate.h"
#include "libmesh/int_range.h"

registerMooseObject("TensorMechanicsApp", CrystalPlasticityHCPDislocationSlipBeyerleinUpdate);

InputParameters
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::validParams()
{
  InputParameters params = CrystalPlasticityStressUpdateBase::validParams();
  params.addClassDescription("Two-term dislocation slip model for hexagonal close packed crystals "
                             "from Beyerline and Tome");

  params.set<MooseEnum>("crystal_lattice_type") = "HCP";
  params.suppressParameter<MooseEnum>("crystal_lattice_type");

  params.addCoupledVar("temperature", "The name of the temperature variable");
  params.addRequiredRangeCheckedParam<Real>(
      "initial_forest_dislocation_density",
      "initial_forest_dislocation_density>0",
      "The initial density of the forest dislocations, in 1/mm^2, assumed "
      "to be split evenly among all slip systems");
  params.addRequiredRangeCheckedParam<Real>(
      "initial_substructure_density",
      "initial_substructure_density>0",
      "The initial total density of the sessile dislocations, in 1/mm^2");

  params.addParam<unsigned int>(
      "slip_system_modes",
      1,
      "Number of different types of slip systems in this HCP crystal, e.g. for a material with "
      "basal<a>, prismatic<a>, and pyramidal<a> active slip systems, this number would be 3");
  params.addParam<std::vector<unsigned int>>(
      "number_slip_systems_per_mode",
      std::vector<unsigned int>(),
      "The number of slip systems per each slip system type. The sum of the entries of the vector "
      "given here must equal the value given for the total number of slip systems.");
  params.addParam<std::vector<Real>>(
      "lattice_friction_per_mode",
      std::vector<Real>(),
      "Value of the lattice friction for each type of the slip system, units of MPa. The order "
      "must be consistent with the number of slip systems per type vector.");

  params.addParam<std::vector<Real>>(
      "effective_shear_modulus_per_mode",
      std::vector<Real>(),
      "Effective isotropic shear modulus value, mu, in MPa. The order "
      "must be consistent with the number of slip systems per type vector.");

  params.addParam<std::vector<Real>>(
      "burgers_vector_per_mode",
      std::vector<Real>(),
      "Value of the Burgers vector, b,  for each type of the slip system, units of mm. The order "
      "must "
      "be consistent with the number of slip systems per type vector.");
  params.addParam<std::vector<Real>>(
      "slip_generation_coefficient_per_mode",
      std::vector<Real>(),
      "Slip dislocation generation coefficient value for each type of the slip system, k_1, units "
      "of 1/mm. The order "
      "must be consistent with the number of slip systems per type vector.");
  params.addParam<std::vector<Real>>(
      "normalized_slip_activiation_energy_per_mode",
      std::vector<Real>(),
      "Value of the slip dislocation attraction activation energy for each type of the slip "
      "system, g, dimensionless. The order must be consistent with the number of slip systems per "
      "type vector.");
  params.addParam<std::vector<Real>>(
      "slip_energy_proportionality_factor_per_mode",
      std::vector<Real>(),
      "Value of the the dislocation slip attraction energy proportionality factor for each type of "
      "the slip system, D, units of MPa. The order must be consistent with the number of slip "
      "systems "
      "per type vector.");
  params.addParam<std::vector<Real>>(
      "substructure_rate_coefficient_per_mode",
      std::vector<Real>(),
      "Material-independent rate constant that accounts for locking of slip dislocations in "
      "sessile substructure dislocation segments, q, dimensionless. This value is often determined "
      "through dislocation dynamics calculations. The order must be consistent with the number of "
      "slip systems per type vector.");

  params.addParam<Real>("gamma_o", 1.0e-3, "Reference strain rate on each slip system, in 1/s");
  params.addParam<Real>("strain_rate_sensitivity_exponent",
                        0.05,
                        "The strain rate sensitivity exponent for the power law relationship of "
                        "resolved shear stress");
  params.addParam<Real>("forest_interaction_parameter",
                        0.9,
                        "Forest dislocation interaction parameter, Chi, dimensionless.");
  params.addParam<Real>("Boltzman_constant", 1.38065e-20, "Boltzman constant, in MPa-mm^3/K");
  params.addRangeCheckedParam<Real>("applied_strain_rate",
                                    1.0e-4,
                                    "applied_strain_rate<=1.0e-3 & applied_strain_rate>=1.0e-5",
                                    "Value of the applied macroscopic strain rate and should "
                                    "correspond to the simulation loading conditions, in 1/s.");
  params.addParam<Real>("reference_macroscopic_strain_rate",
                        1.0e7,
                        "Value of the reference macroscopic strain rate for the thermal "
                        "dislocation attraction, in 1/s.");
  params.addParam<Real>("substructure_hardening_coefficient",
                        0.086,
                        "Value of the coefficient for the expanded Taylor hardening substructure "
                        "hardening relation, set to recover the Taylor hardening law for low "
                        "substructure densities, k_{sub}, dimensionless.");
  params.addParam<std::vector<Real>>(
      "Hall_Petch_like_constant_per_mode",
      std::vector<Real>(),
      "The microstructure Hall-Petch like coefficient value used to capture the influence of grain "
      "size on slip system resistance in the absence of twin dislocations, dimensionless");
  params.addRequiredRangeCheckedParam<Real>(
      "grain_size", "grain_size>0", "Value of the crystal grain size, in mm");

  params.addParam<MaterialPropertyName>(
      "total_twin_volume_fraction",
      "Total twin volume fraction, if twinning is considered in the simulation");

  return params;
}

CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::
    CrystalPlasticityHCPDislocationSlipBeyerleinUpdate(const InputParameters & parameters)
  : CrystalPlasticityStressUpdateBase(parameters),

    _temperature(coupledValue("temperature")),
    _forest_dislocation_density(
        declareProperty<std::vector<Real>>(_base_name + "forest_dislocation_density")),
    _forest_dislocation_density_old(
        getMaterialPropertyOld<std::vector<Real>>(_base_name + "forest_dislocation_density")),
    _forest_dislocation_increment(
        declareProperty<std::vector<Real>>(_base_name + "forest_dislocation_increment")),
    _forest_dislocations_removed_increment(
        declareProperty<std::vector<Real>>(_base_name + "_forest_dislocations_removed_increment")),
    _initial_forest_dislocation_density(getParam<Real>("initial_forest_dislocation_density")),
    _total_substructure_density(declareProperty<Real>(_base_name + "total_substructure_density")),
    _total_substructure_density_old(
        getMaterialPropertyOld<Real>(_base_name + "total_substructure_density")),
    _total_substructure_density_increment(
        declareProperty<Real>(_base_name + "total_substructure_increment")),
    _initial_substructure_density(getParam<Real>("initial_substructure_density")),

    _slip_system_modes(getParam<unsigned int>("slip_system_modes")),
    _number_slip_systems_per_mode(
        getParam<std::vector<unsigned int>>("number_slip_systems_per_mode")),
    _lattice_friction(getParam<std::vector<Real>>("lattice_friction_per_mode")),

    _reference_strain_rate(getParam<Real>("gamma_o")),
    _rate_sensitivity_exponent(getParam<Real>("strain_rate_sensitivity_exponent")),

    _burgers_vector(getParam<std::vector<Real>>("burgers_vector_per_mode")),
    _slip_generation_coefficient(
        getParam<std::vector<Real>>("slip_generation_coefficient_per_mode")),
    _slip_activation_energy(
        getParam<std::vector<Real>>("normalized_slip_activiation_energy_per_mode")),
    _proportionality_factor(
        getParam<std::vector<Real>>("slip_energy_proportionality_factor_per_mode")),
    _forest_interaction_coefficient(getParam<Real>("forest_interaction_parameter")),
    _boltzman_constant(getParam<Real>("Boltzman_constant")),
    _macro_applied_strain_rate(getParam<Real>("applied_strain_rate")),
    _macro_reference_strain_rate(getParam<Real>("reference_macroscopic_strain_rate")),

    _shear_modulus(getParam<std::vector<Real>>("effective_shear_modulus_per_mode")),
    _substructure_rate_coefficient(
        getParam<std::vector<Real>>("substructure_rate_coefficient_per_mode")),
    _substructure_hardening_coefficient(getParam<Real>("substructure_hardening_coefficient")),
    _hallpetch_like_coefficient(getParam<std::vector<Real>>("Hall_Petch_like_constant_per_mode")),
    _grain_size(getParam<Real>("grain_size")),

    // Twinning contributions, if used
    _include_twinning_in_Lp(parameters.isParamValid("total_twin_volume_fraction")),
    _twin_volume_fraction_total(_include_twinning_in_Lp
                                    ? &getMaterialPropertyOld<Real>("total_twin_volume_fraction")
                                    : nullptr)
{
  // resize local caching vectors used for substepping
  _previous_substep_slip_resistance.resize(_number_slip_systems);
  _previous_substep_forest_dislocations.resize(_number_slip_systems);
  _slip_resistance_before_update.resize(_number_slip_systems);
  _forest_dislocations_before_update.resize(_number_slip_systems);

  // check that the number of slip systems is equal to the sum of the types of slip system
  if (_number_slip_systems_per_mode.size() != _slip_system_modes)
    paramError("number_slip_systems_per_mode",
               "The size the number of slip systems per mode is not equal to the number of slip "
               "system types.");

  // Check that the number of slip mode dependent parameters is given matches the number of slip
  // modes
  if (_burgers_vector.size() != _slip_system_modes)
    paramError("burgers_vector_per_mode",
               "Please ensure that the size of burgers_vector_per_mode equals the value supplied "
               "for slip_system_modes");

  if (_slip_generation_coefficient.size() != _slip_system_modes)
    paramError("slip_generation_coefficient_per_mode",
               "Please ensure that the size of slip_generation_coefficient_per_mode equals the "
               "value supplied for slip_system_modes");

  if (_slip_activation_energy.size() != _slip_system_modes)
    paramError("normalized_slip_activiation_energy_per_mode",
               "Please ensure that the size of normalized_slip_activiation_energy_per_mode equals "
               "the value supplied for slip_system_modes");

  if (_proportionality_factor.size() != _slip_system_modes)
    paramError("slip_energy_proportionality_factor_per_mode",
               "Please ensure that the size of slip_energy_proportionality_factor_per_mode equals "
               "the value supplied for slip_system_modes");

  if (_shear_modulus.size() != _slip_system_modes)
    paramError("effective_shear_modulus_per_mode",
               "Please ensure that the size of effective_shear_modulus_per_mode equals the "
               "value supplied for slip_system_modes");

  if (_substructure_rate_coefficient.size() != _slip_system_modes)
    paramError("substructure_rate_coefficient_per_mode",
               "Please ensure that the size of substructure_rate_coefficient_per_mode equals the "
               "value supplied for slip_system_modes");

  if (_hallpetch_like_coefficient.size() != _slip_system_modes)
    paramError("Hall_Petch_like_constant_per_mode",
               "Please ensure that the size of Hall_Petch_like_constant_per_mode equals the value "
               "supplied for slip_system_modes");

  if (_lattice_friction.size() != _slip_system_modes)
    paramError("lattice_friction_per_mode",
               "Please ensure that the size of lattice_friction_per_mode equals the value supplied "
               "for slip_system_modes");

  unsigned int sum = 0;
  for (const auto i : make_range(_slip_system_modes))
    sum += _number_slip_systems_per_mode[i];
  if (sum != _number_slip_systems)
    paramError("slip_system_modes",
               "The number of slip systems and the sum of the slip systems in each of the slip "
               "system modes are not equal");
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::initQpStatefulProperties()
{
  CrystalPlasticityStressUpdateBase::initQpStatefulProperties();

  // Resize constitutive-model specific material properties
  _forest_dislocation_density[_qp].resize(_number_slip_systems);
  _forest_dislocation_increment[_qp].resize(_number_slip_systems);
  _forest_dislocations_removed_increment[_qp].resize(_number_slip_systems);
  _initial_lattice_friction.resize(_number_slip_systems);

  // Set constitutive-model specific initial values from parameters
  const Real forest_density_per_system = _initial_forest_dislocation_density / _number_slip_systems;
  for (const auto i : make_range(_number_slip_systems))
  {
    _forest_dislocation_density[_qp][i] = forest_density_per_system;
    _forest_dislocation_increment[_qp][i] = 0.0;
    _slip_increment[_qp][i] = 0.0;
  }

  // Set initial resistance from lattice friction, which is type dependent
  unsigned int slip_mode = 0;
  unsigned int counter_adjustment = 0;
  for (const auto i : make_range(_number_slip_systems))
  {
    if ((i - counter_adjustment) < _number_slip_systems_per_mode[slip_mode])
      _initial_lattice_friction(i) = _lattice_friction[slip_mode];
    else
    {
      counter_adjustment += _number_slip_systems_per_mode[slip_mode];
      ++slip_mode;
      _initial_lattice_friction(i) = _lattice_friction[slip_mode];
    }
  }

  calculateGrainSizeResistance();

  for (const auto i : make_range(_number_slip_systems))
    _slip_resistance[_qp][i] = _initial_lattice_friction(i);

  _total_substructure_density[_qp] = _initial_substructure_density;
  _total_substructure_density_increment[_qp] = 0.0;
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateGrainSizeResistance()
{
  unsigned int slip_mode = 0;
  unsigned int counter_adjustment = 0;
  for (const auto i : make_range(_number_slip_systems))
  {
    Real hallpetch_burgers_term = 0.0;
    if ((i - counter_adjustment) < _number_slip_systems_per_mode[slip_mode])
      hallpetch_burgers_term = _hallpetch_like_coefficient[slip_mode] * _shear_modulus[slip_mode] *
                               std::sqrt(_burgers_vector[slip_mode]);
    else
    {
      counter_adjustment += _number_slip_systems_per_mode[slip_mode];
      ++slip_mode;
      hallpetch_burgers_term = _hallpetch_like_coefficient[slip_mode] * _shear_modulus[slip_mode] *
                               std::sqrt(_burgers_vector[slip_mode]);
    }
    _initial_lattice_friction(i) += hallpetch_burgers_term / std::sqrt(_grain_size);
  }
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::setInitialConstitutiveVariableValues()
{
  _slip_resistance[_qp] = _slip_resistance_old[_qp];
  _previous_substep_slip_resistance = _slip_resistance_old[_qp];

  _forest_dislocation_density[_qp] = _forest_dislocation_density_old[_qp];
  _previous_substep_forest_dislocations = _forest_dislocation_density_old[_qp];

  _total_substructure_density[_qp] = _total_substructure_density_old[_qp];
  _previous_substep_total_substructure_density = _total_substructure_density_old[_qp];
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::setSubstepConstitutiveVariableValues()
{
  _slip_resistance[_qp] = _previous_substep_slip_resistance;
  _forest_dislocation_density[_qp] = _previous_substep_forest_dislocations;
  _total_substructure_density[_qp] = _previous_substep_total_substructure_density;
}

bool
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateSlipRate()
{
  for (const auto i : make_range(_number_slip_systems))
  {
    Real driving_force = std::abs(_tau[_qp][i] / _slip_resistance[_qp][i]);
    if (driving_force < _zero_tol)
      _slip_increment[_qp][i] = 0.0;
    else
    {
      _slip_increment[_qp][i] =
          _reference_strain_rate * std::pow(driving_force, (1.0 / _rate_sensitivity_exponent));
      if (_tau[_qp][i] < 0.0)
        _slip_increment[_qp][i] *= -1.0;
    }
    if (std::abs(_slip_increment[_qp][i]) * _substep_dt > _slip_incr_tol)
    {
      if (_print_convergence_message)
        mooseWarning("Maximum allowable slip increment exceeded ",
                     std::abs(_slip_increment[_qp][i]) * _substep_dt);
      return false;
    }
  }
  return true;
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateEquivalentSlipIncrement(
    RankTwoTensor & equivalent_slip_increment)
{
  if (_include_twinning_in_Lp)
  {
    for (const auto i : make_range(_number_slip_systems))
      equivalent_slip_increment += (1.0 - (*_twin_volume_fraction_total)[_qp]) *
                                   _flow_direction[_qp][i] * _slip_increment[_qp][i] * _substep_dt;
  }
  else // if no twinning volume fraction material property supplied, use base class
    CrystalPlasticityStressUpdateBase::calculateEquivalentSlipIncrement(equivalent_slip_increment);
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateConstitutiveSlipDerivative(
    std::vector<Real> & dslip_dtau)
{
  for (const auto i : make_range(_number_slip_systems))
  {
    if (MooseUtils::absoluteFuzzyEqual(_tau[_qp][i], 0.0))
      dslip_dtau[i] = 0.0;
    else
      dslip_dtau[i] = _slip_increment[_qp][i] /
                      (_rate_sensitivity_exponent * std::abs(_tau[_qp][i])) * _substep_dt;
  }
}

bool
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::areConstitutiveStateVariablesConverged()
{
  if (isConstitutiveStateVariableConverged(_forest_dislocation_density[_qp],
                                           _forest_dislocations_before_update,
                                           _previous_substep_forest_dislocations,
                                           _rel_state_var_tol) &&
      isSubstructureDislocationDensityConverged() &&
      isConstitutiveStateVariableConverged(_slip_resistance[_qp],
                                           _slip_resistance_before_update,
                                           _previous_substep_slip_resistance,
                                           _resistance_tol))
    return true;
  return false;
}

bool
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::isSubstructureDislocationDensityConverged()
{
  bool converged_flag = true;

  Real substructure_diff =
      std::abs(_total_substructure_density_before_update - _total_substructure_density[_qp]);

  if (_previous_substep_total_substructure_density < _zero_tol && substructure_diff > _zero_tol)
    converged_flag = false;
  else if (_previous_substep_total_substructure_density > _zero_tol &&
           substructure_diff > _rel_state_var_tol * _previous_substep_total_substructure_density)
    converged_flag = false;

  return converged_flag;
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::updateSubstepConstitutiveVariableValues()
{
  _previous_substep_slip_resistance = _slip_resistance[_qp];
  _previous_substep_forest_dislocations = _forest_dislocation_density[_qp];
  _previous_substep_total_substructure_density = _total_substructure_density[_qp];
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::cacheStateVariablesBeforeUpdate()
{
  _slip_resistance_before_update = _slip_resistance[_qp];
  _forest_dislocations_before_update = _forest_dislocation_density[_qp];
  _total_substructure_density_before_update = _total_substructure_density[_qp];
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateStateVariableEvolutionRateComponent()
{
  calculateForestDislocationEvolutionIncrement();
  calculateSubstructureDensityEvolutionIncrement();
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateForestDislocationEvolutionIncrement()
{
  DenseVector<Real> k1_term(_number_slip_systems);
  DenseVector<Real> k2_term(_number_slip_systems);

  const Real temperature_strain_term =
      _boltzman_constant * _temperature[_qp] *
      std::log(_macro_applied_strain_rate / _macro_reference_strain_rate);

  // solve first for the coefficients, which depend on the given slip mode
  unsigned int slip_mode = 0;
  unsigned int counter_adjustment = 0;
  for (const auto i : make_range(_number_slip_systems))
  {
    Real interaction_term = 0.0;
    Real volume_term = 0.0;
    if ((i - counter_adjustment) < _number_slip_systems_per_mode[slip_mode])
    {
      k1_term(i) = _slip_generation_coefficient[slip_mode];
      interaction_term = _forest_interaction_coefficient * _burgers_vector[slip_mode] /
                         _slip_activation_energy[slip_mode];
      volume_term =
          _proportionality_factor[slip_mode] * Utility::pow<3>(_burgers_vector[slip_mode]);
    }
    else
    {
      counter_adjustment += _number_slip_systems_per_mode[slip_mode];
      ++slip_mode;

      k1_term(i) = _slip_generation_coefficient[slip_mode];
      interaction_term = _forest_interaction_coefficient * _burgers_vector[slip_mode] /
                         _slip_activation_energy[slip_mode];
      volume_term =
          _proportionality_factor[slip_mode] * Utility::pow<3>(_burgers_vector[slip_mode]);
    }
    k2_term(i) = interaction_term * k1_term(i) * (1.0 - temperature_strain_term / volume_term);
  }

  for (const auto i : make_range(_number_slip_systems))
  {
    const Real abs_slip_increment = std::abs(_slip_increment[_qp][i]);
    Real generated_dislocations = 0.0;

    if (_forest_dislocation_density[_qp][i] > 0.0)
      generated_dislocations = k1_term(i) * std::sqrt(_forest_dislocation_density[_qp][i]) *
                               abs_slip_increment * _substep_dt;

    _forest_dislocations_removed_increment[_qp][i] =
        k2_term(i) * _forest_dislocation_density[_qp][i] * abs_slip_increment * _substep_dt;

    _forest_dislocation_increment[_qp][i] =
        generated_dislocations - _forest_dislocations_removed_increment[_qp][i];
  }
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateSubstructureDensityEvolutionIncrement()
{
  // calculate the generation coefficient, which depends on the slip mode
  DenseVector<Real> generation_term(_number_slip_systems);

  unsigned int slip_mode = 0;
  unsigned int counter_adjustment = 0;
  for (const auto i : make_range(_number_slip_systems))
  {
    if ((i - counter_adjustment) < _number_slip_systems_per_mode[slip_mode])
      generation_term(i) = _substructure_rate_coefficient[slip_mode] * _burgers_vector[slip_mode];
    else
    {
      counter_adjustment += _number_slip_systems_per_mode[slip_mode];
      ++slip_mode;
      generation_term(i) = _substructure_rate_coefficient[slip_mode] * _burgers_vector[slip_mode];
    }
  }

  // perform the summing calculation over all slip systems
  _total_substructure_density_increment[_qp] = 0.0;
  const Real sqrt_substructures = std::sqrt(_total_substructure_density[_qp]);

  for (const auto i : make_range(_number_slip_systems))
    _total_substructure_density_increment[_qp] +=
        generation_term(i) * sqrt_substructures * _forest_dislocations_removed_increment[_qp][i];
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateSlipResistance()
{
  DenseVector<Real> forest_hardening(_number_slip_systems);
  DenseVector<Real> substructure_hardening(_number_slip_systems);

  unsigned int slip_mode = 0;
  unsigned int counter_adjustment = 0;
  for (const auto i : make_range(_number_slip_systems))
  {
    Real burgers = 0.0;
    Real shear_modulus = 0.0;
    if ((i - counter_adjustment) < _number_slip_systems_per_mode[slip_mode])
    {
      burgers = _burgers_vector[slip_mode];
      shear_modulus = _shear_modulus[slip_mode];
    }
    else
    {
      counter_adjustment += _number_slip_systems_per_mode[slip_mode];
      ++slip_mode;
      burgers = _burgers_vector[slip_mode];
      shear_modulus = _shear_modulus[slip_mode];
    }

    // forest dislocation hardening
    if (_forest_dislocation_density[_qp][i] > 0.0)
      forest_hardening(i) = _forest_interaction_coefficient * burgers * shear_modulus *
                            std::sqrt(_forest_dislocation_density[_qp][i]);
    else
      forest_hardening(i) = 0.0;

    // substructure dislocation hardening
    if (_total_substructure_density[_qp] > 0.0)
    {
      const Real spacing_term = burgers * std::sqrt(_total_substructure_density[_qp]);
      substructure_hardening(i) = _substructure_hardening_coefficient * shear_modulus *
                                  spacing_term * std::log10(1.0 / spacing_term);
    }
    else
      substructure_hardening(i) = 0.0;
  }

  // have the constant initial value, while it's not a function of temperature, sum
  for (const auto i : make_range(_number_slip_systems))
    _slip_resistance[_qp][i] =
        _initial_lattice_friction(i) + forest_hardening(i) + substructure_hardening(i);
}

bool
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::updateStateVariables()
{
  if (calculateForestDislocationDensity() && calculateSubstructureDislocationDensity())
    return true;
  else
    return false;
}

bool
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateForestDislocationDensity()
{
  for (const auto i : make_range(_number_slip_systems))
  {
    if (_previous_substep_forest_dislocations[i] < _zero_tol &&
        _forest_dislocation_increment[_qp][i] < 0.0)
      _forest_dislocation_density[_qp][i] = _previous_substep_forest_dislocations[i];
    else
      _forest_dislocation_density[_qp][i] =
          _previous_substep_forest_dislocations[i] + _forest_dislocation_increment[_qp][i];

    if (_forest_dislocation_density[_qp][i] < 0.0)
      return false;
  }
  return true;
}

bool
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateSubstructureDislocationDensity()
{
  if (_previous_substep_total_substructure_density < _zero_tol &&
      _total_substructure_density_increment[_qp] < 0.0)
    _total_substructure_density[_qp] = _previous_substep_total_substructure_density;
  else
    _total_substructure_density[_qp] =
        _previous_substep_total_substructure_density + _total_substructure_density_increment[_qp];

  if (_total_substructure_density[_qp] < 0.0)
    return false;

  return true;
}
