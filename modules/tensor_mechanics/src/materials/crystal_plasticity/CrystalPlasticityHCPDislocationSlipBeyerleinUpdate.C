//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrystalPlasticityHCPDislocationSlipBeyerleinUpdate.h"

registerMooseObject("TensorMechanicsApp", CrystalPlasticityHCPDislocationSlipBeyerleinUpdate);

InputParameters
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::validParams()
{
  InputParameters params = CrystalPlasticityStressUpdateBase::validParams();
  params.addClassDescription("Two-term dislocation slip model for hexagonal close packed crystals "
                             "from Beyerline and Tome");

  params.set<MooseEnum>("unit_cell_type") = "HCP";
  params.suppressParameter<MooseEnum>("unit_cell_type");

  params.addCoupledVar("temperature", "The name of the temperature variable");
  params.addRangeCheckedParam("initial_forest_dislocation_density",
                              0.0,
                              "initial_forest_dislocation_density>0",
                              "The initial density of the forest dislocations, in 1/mm^2, assumed "
                              "to be split evenly among all slip systems");
  params.addRangeCheckedParam("initial_substructure_density",
                              0.0,
                              "initial_substructure_density>=0",
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
  // TODO: need to add function capability to have temperature dependence to lattice friction

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
  // TODO: need to add function capability to have temperature dependence to shear modulus value
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
  params.addParam<Real>("grain_size", 0.0, "Value of the crystal grain size, in mm");

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
    // TODO: need to add function capability to have temperature dependence to lattice friction

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
    // TODO: setup the temperature dependence through a function capability
    _substructure_rate_coefficient(
        getParam<std::vector<Real>>("substructure_rate_coefficient_per_mode")),
    _substructure_hardening_coefficient(getParam<Real>("substructure_hardening_coefficient")),
    _hallpetch_like_coefficient(getParam<std::vector<Real>>("Hall_Petch_like_constant_per_mode")),
    _grain_size(getParam<Real>("grain_size"))
{
  // resize local caching vectors used for substepping
  _previous_substep_slip_resistance.resize(_number_slip_systems);
  _previous_substep_forest_dislocations.resize(_number_slip_systems);
  _slip_resistance_before_update.resize(_number_slip_systems);
  _forest_dislocations_before_update.resize(_number_slip_systems);
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::initQpStatefulProperties()
{
  // check that the number of slip systems is equal to the sum of the types of slip system
  if (_number_slip_systems_per_mode.size() != _slip_system_modes)
    mooseError("CrystalPlasticityHCPDislocationSlipBeyerleinUpdate Error: The size the number of "
               "slip systems per type is not equal to the number of slip system types.");

  unsigned int sum = 0;
  for (unsigned int i = 0; i < _slip_system_modes; ++i)
    sum += _number_slip_systems_per_mode[i];
  if (sum != _number_slip_systems)
    mooseError("The number of slip systems and the sum of the slip systems in each of the slip "
               "system types are not equal");

  CrystalPlasticityStressUpdateBase::initQpStatefulProperties();

  // Resize constitutive-model specific material properties
  _forest_dislocation_density[_qp].resize(_number_slip_systems);
  _forest_dislocation_increment[_qp].resize(_number_slip_systems);
  _forest_dislocations_removed_increment[_qp].resize(_number_slip_systems);
  _initial_lattice_friction.resize(_number_slip_systems);

  // Set constitutive-model specific initial values from parameters
  const Real forest_density_per_system = _initial_forest_dislocation_density / _number_slip_systems;
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    _forest_dislocation_density[_qp][i] = forest_density_per_system;
    _forest_dislocation_increment[_qp][i] = 0.0;
    _slip_increment[_qp][i] = 0.0;
  }

  // Set initial resistance from lattice friction, which is type dependent
  unsigned int slip_mode = 0;
  unsigned int counter_adjustment = 0;
  // std::cout << "\nCheck the value of the lattice friction only slip resistance: \n";
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    if ((i - counter_adjustment) < _number_slip_systems_per_mode[slip_mode])
      _initial_lattice_friction(i) = _lattice_friction[slip_mode];
    else
    {
      counter_adjustment += _number_slip_systems_per_mode[slip_mode];
      ++slip_mode;
      if (slip_mode >= _slip_system_modes)
        mooseError("Error in accessing the entries of the lattice_friction_per_mode vector. "
                   "Please ensure that the size of lattice_friction_per_mode equals the value "
                   "supplied for slip_system_modes");
      else
        _initial_lattice_friction(i) = _lattice_friction[slip_mode];
    }
    // std::cout << "  at slip system number " << i << " the value of the slip resistance is " << _initial_lattice_friction(i) << "\n";
  }

  calculateGrainSizeResistance();

  for (unsigned int i=0; i < _number_slip_systems; ++i)
    _slip_resistance[_qp][i] = _initial_lattice_friction(i);

  _total_substructure_density[_qp] = 0.0;
  _total_substructure_density_increment[_qp] = 0.0;
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateGrainSizeResistance()
{
  // std::cout << "\n Within the calculateGrainSizeResistance method: \n";
  unsigned int slip_mode = 0;
  unsigned int counter_adjustment = 0;
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
      Real hallpetch_burgers_term = 0.0;
    if ((i - counter_adjustment) < _number_slip_systems_per_mode[slip_mode])
      hallpetch_burgers_term = _hallpetch_like_coefficient[slip_mode] * _shear_modulus[slip_mode] * std::sqrt(_burgers_vector[slip_mode]);
    else
    {
      counter_adjustment += _number_slip_systems_per_mode[slip_mode];
      ++slip_mode;
      if (slip_mode >= _slip_system_modes)
        mooseError("Error in accessing the entries of the Hall_Petch_like_constant_per_mode "
                   "vector. Please ensure that the size of Hall_Petch_like_constant_per_mode "
                   "equals the value supplied for slip_system_modes");
      else
        hallpetch_burgers_term = _hallpetch_like_coefficient[slip_mode] * _shear_modulus[slip_mode] * std::sqrt(_burgers_vector[slip_mode]);
    }
    // std::cout << "  on slip system number " << i << " the value of the hallpetch term is " << hallpetch_burgers_term <<"\n";
    _initial_lattice_friction(i) += hallpetch_burgers_term / std::sqrt(_grain_size);
    // std::cout << "  and the updated initial lattice friction value is " << _initial_lattice_friction(i) << "\n";
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
  // std::cout << "At the beginning of the calculateSlipRate method: \n";
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    // std::cout << "On slip system number " << i << ":\n";
    // std::cout << "   the applied resolved stress is " << _tau[_qp][i] << "\n";
    // std::cout << "   the slip resistance is " << _slip_resistance[_qp][i] << "\n";
    // std::cout << "   and the reference strain rate value is " << _reference_strain_rate << "\n";
    Real driving_force = std::abs(_tau[_qp][i] / _slip_resistance[_qp][i]);
    _slip_increment[_qp][i] =
        _reference_strain_rate * std::pow(driving_force, (1.0 / _rate_sensitivity_exponent));
    if (_tau[_qp][i] < 0.0)
      _slip_increment[_qp][i] *= -1.0;

    // std::cout << "  then the calculated slip increment value is: " << _slip_increment[_qp][i] << "\n\n";

    if (std::abs(_slip_increment[_qp][i]) * _substep_dt > _slip_incr_tol)
    {
#ifdef DEBUG
      mooseWarning("Maximum allowable slip increment exceeded ",
                   std::abs(_slip_increment[_qp][i]) * _substep_dt);
#endif
      return false;
    }
  }
  return true;
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateConstitutiveSlipDerivative(
    std::vector<Real> & dslip_dtau)
{
  calculateSlipDerivative(dslip_dtau);
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateSlipDerivative(
    std::vector<Real> & ddislocaitonslip_dtau)
{
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    if (MooseUtils::absoluteFuzzyEqual(_tau[_qp][i], 0.0))
      ddislocaitonslip_dtau[i] = 0.0;
    else
      ddislocaitonslip_dtau[i] = _slip_increment[_qp][i] /
                                 (_rate_sensitivity_exponent * std::abs(_tau[_qp][i])) *
                                 _substep_dt;
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

  const Real relative_strain_rate =
      std::log(_macro_applied_strain_rate / _macro_reference_strain_rate);
  const Real temperature_strain_term =
      _boltzman_constant * _temperature[_qp] * relative_strain_rate;

  // solve first for the coefficients, which depend on the given slip mode
  unsigned int slip_mode = 0;
  unsigned int counter_adjustment = 0;
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
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
      if (slip_mode >= _slip_system_modes)
        mooseError(
            "Error in accessing either the entries of the slip_generation_coefficient_per_mode, "
            "Burgers_vector_per_mode, normalized_slip_activiation_energy_per_mode, or "
            "slip_energy_proportionality_factor_per_mode vectors. Please ensure that the size of "
            "these vectors equals the value supplied for slip_system_modes");
      else
      {
        k1_term(i) = _slip_generation_coefficient[slip_mode];
        interaction_term = _forest_interaction_coefficient * _burgers_vector[slip_mode] /
                           _slip_activation_energy[slip_mode];
        volume_term =
            _proportionality_factor[slip_mode] * Utility::pow<3>(_burgers_vector[slip_mode]);
      }
    }
    k2_term(i) = interaction_term * k1_term(i) * (1.0 - temperature_strain_term / volume_term);
  }

  // Loop over the number of slip systems to solve for the dislocation density increments
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    _forest_dislocations_removed_increment[_qp][i] =
        k2_term(i) * _forest_dislocation_density[_qp][i] * _slip_increment[_qp][i] * _substep_dt;
    _forest_dislocation_increment[_qp][i] = k1_term(i) *
                                                std::sqrt(_forest_dislocation_density[_qp][i]) *
                                                _slip_increment[_qp][i] * _substep_dt -
                                            _forest_dislocations_removed_increment[_qp][i];
  }
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateSubstructureDensityEvolutionIncrement()
{
  // calculate the generation coefficient, which depends on the slip mode
  DenseVector<Real> generation_term(_number_slip_systems);

  unsigned int slip_mode = 0;
  unsigned int counter_adjustment = 0;
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    if ((i - counter_adjustment) < _number_slip_systems_per_mode[slip_mode])
      generation_term(i) = _substructure_rate_coefficient[slip_mode] * _burgers_vector[slip_mode];
    else
    {
      counter_adjustment += _number_slip_systems_per_mode[slip_mode];
      ++slip_mode;
      if (slip_mode >= _slip_system_modes)
        mooseError(
            "Error in accessing either the entries of the substructure_rate_coefficient_per_mode "
            "or the Burgers_vector_per_mode vectors. Please ensure that the size "
            "of these vectors equals the value supplied for slip_system_modes");
      else
        generation_term(i) = _substructure_rate_coefficient[slip_mode] * _burgers_vector[slip_mode];
    }
  }

  // perform the summing calculation over all slip systems
  _total_substructure_density_increment[_qp] = 0.0;
  const Real sqrt_substructures = std::sqrt(_total_substructure_density[_qp]);

  for (unsigned int i = 0; i < _number_slip_systems; ++i)
    _total_substructure_density_increment[_qp] +=
        generation_term(i) * sqrt_substructures * _forest_dislocations_removed_increment[_qp][i];
}

void
CrystalPlasticityHCPDislocationSlipBeyerleinUpdate::calculateSlipResistance()
{
  DenseVector<Real> forest_hardening(_number_slip_systems);
  DenseVector<Real> substructure_hardening(_number_slip_systems);

  // std::cout << "Within the calculate slip resistance method: \n";

  unsigned int slip_mode = 0;
  unsigned int counter_adjustment = 0;
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    // std::cout << "On the slip system number " << i << " :\n";
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
      if (slip_mode >= _slip_system_modes)
        mooseError(
            "Error in accessing the entries of the Burgers_vector_per_mode vectors. Please ensure "
            "that the size of these vectors equals the value supplied for slip_system_modes");
      else
      {
        burgers = _burgers_vector[slip_mode];
        shear_modulus = _shear_modulus[slip_mode];
      }
    }

    // forest dislocation hardening
    const Real coeff = _forest_interaction_coefficient * burgers * shear_modulus;
    forest_hardening(i) = coeff * std::sqrt(_forest_dislocation_density[_qp][i]);
    // std::cout << "  the forest hardening cofficient value is " << coeff << "\n";
    // std::cout << "  where the forest interaction coefficient is " << _forest_interaction_coefficient << "\n";
    // std::cout << "  the Burgers vector value is " << burgers << "\n";
    // std::cout << "  the shear modulus value is " << shear_modulus << "\n";
    // std::cout << "  and the forest hardening value is " << forest_hardening(i) << "\n";

    // substructure dislocation hardening
    if (_total_substructure_density[_qp] > 0.0)
      {
        const Real spacing_term = burgers * std::sqrt(_total_substructure_density[_qp]);
        // std::cout << "  The substructure coefficient term is " << spacing_term << "\n";
        substructure_hardening(i) = _substructure_hardening_coefficient * shear_modulus *
                                    spacing_term * std::log10(spacing_term);
      }
    else
      substructure_hardening(i) = 0.0;

    // std::cout << "  and the value of the substructure hardening value is " << substructure_hardening(i) << "\n\n";
  }

  // have the constant initial value, while it's not a function of temperature, sum
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    _slip_resistance[_qp][i] =
        _initial_lattice_friction(i) + forest_hardening(i) + substructure_hardening(i);
    // std::cout << "\nOn slip system number " << i << "\n";
    // std::cout << "  Then the final slip resistance value is " << _slip_resistance[_qp][i] << "\n";
    // std::cout << "  where the initial lattice friction value is " << _initial_lattice_friction(i) << "\n";
    // std::cout << "  the forest hardening value is " << forest_hardening(i) << "\n";
    // std::cout << "  and the substructure_hardening value is " << substructure_hardening(i) << "\n";
  }
  // std::cout << "\n";
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
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
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
