//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrystalPlasticityFCCDislocationLinkHuCocksUpdate.h"
#include "libmesh/int_range.h"

registerMooseObject("TensorMechanicsApp", CrystalPlasticityFCCDislocationLinkHuCocksUpdate);

InputParameters
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::validParams()
{
  InputParameters params = CrystalPlasticityStressUpdateBase::validParams();
  params.addClassDescription(
      "Athermal dislocation glide model for FCC cyrstals based on dislocation pinning points on "
      "slip planes. This model was developed for stainless steel by Hu and Cocks.");

  params.set<MooseEnum>("crystal_lattice_type") = "FCC";
  params.suppressParameter<MooseEnum>("crystal_lattice_type");
  params.makeParamRequired<unsigned int>("number_coplanar_groups");

  params.addRequiredRangeCheckedParam<Real>(
      "initial_pinning_point_density",
      "initial_pinning_point_density>0",
      "The initial state density of the dislocation pinning points, in 1/mm^2, assumed to be "
      "split "
      "evenly among all slip planes (the number of coplanar groups).");

  params.addParam<MaterialPropertyName>(
      "solute_concentration",
      "The material property name of the solute number density for all the solute atoms in the "
      "material, in 1/mm^3. This value can be computed from the material composition.");
  params.addParam<MaterialPropertyName>(
      "precipitate_number_density",
      "The material property name of the number density of the precipitates, in 1/mm^3.");
  params.addParam<MaterialPropertyName>(
      "mean_precipitate_radius",
      "The material property name for the mean radius value, in mm, for the precipitates");

  params.addRangeCheckedParam<Real>(
      "dislocation_multiplication_coefficient",
      1.0,
      "dislocation_multiplication_coefficient>0",
      "Scaling coefficient for the multiplication terms of the dislocation evolution");

  params.addRangeCheckedParam<Real>("reference_slip_rate",
                                    1.0,
                                    "reference_slip_rate>0",
                                    "reference strain rate on the slip system, in mm/s");
  params.addRangeCheckedParam<Real>("strain_rate_sensitivity_exponent",
                                    600.0,
                                    "strain_rate_sensitivity_exponent>0",
                                    "The strain rate sensitivity exponent for the power law "
                                    "relationship for the dislocation glide velocity");
  params.addRangeCheckedParam<Real>(
      "coefficient_self_plane_evolution",
      1.0,
      "coefficient_self_plane_evolution>=0",
      "The fitting parameter used to compute the evolution of the pinning point density on the "
      "self-hardening slip plane due to multiple dislocation link release, dislocation link "
      "expansion, and dislocation network refinement.");
  params.addRangeCheckedParam<Real>(
      "coefficient_latent_plane_evolution",
      1.0,
      "coefficient_latent_plane_evolution>=0",
      "The fitting parameter used to compute the evolution of the pinning point density on a slip "
      "plane, due to latent hardening on other slip planes from dislocation link network "
      "refinement.");

  params.addRangeCheckedParam<Real>("forest_dislocation_hardening_coefficient",
                                    1.0,
                                    "forest_dislocation_hardening_coefficient>=0",
                                    "Leading coefficient for forest dislocation contribution to "
                                    "the slip system resistance value.");
  params.addRangeCheckedParam<Real>("solute_hardening_coefficient",
                                    1.0,
                                    "solute_hardening_coefficient>=0",
                                    "Leading coefficient for the contribution to the slip system "
                                    "resistance from solid solution solute atoms.");
  params.addRangeCheckedParam<Real>(
      "precipitate_hardening_coefficient",
      1.0,
      "precipitate_hardening_coefficient>=0",
      "Leading coefficient for the precipitate contribution to slip system hardening; precipates "
      "are assumed to act as arrays of weak pinning barriers to dislocation motion.");
  params.addRequiredRangeCheckedParam<Real>(
      "burgers_vector", "burgers_vector>0", "The Burger's vector for the material, in mm");
  params.addRequiredRangeCheckedParam<Real>(
      "shear_modulus",
      "shear_modulus>0",
      "The average shear modulus value, in MPa, used to calculate the "
      "hardening behavior of the individual slip systems");

  params.addParam<MaterialPropertyName>(
      "total_twin_volume_fraction",
      "Total twin volume fraction, if twinning is considered in the simulation");

  return params;
}

CrystalPlasticityFCCDislocationLinkHuCocksUpdate::CrystalPlasticityFCCDislocationLinkHuCocksUpdate(
    const InputParameters & parameters)
  : CrystalPlasticityStressUpdateBase(parameters),

    _pinning_point_density(
        declareProperty<std::vector<Real>>(_base_name + "pinning_point_density")),
    _pinning_point_density_old(
        getMaterialPropertyOld<std::vector<Real>>(_base_name + "pinning_point_density")),
    _pinning_point_increment(
        declareProperty<std::vector<Real>>(_base_name + "dislocation_increment")),
    _initial_pinning_point_density(getParam<Real>(_base_name + "initial_pinning_point_density")),

    // Hardening contributing populations
    _include_solute_hardening(parameters.isParamValid("solute_concentration")),
    _solute_concentration(_include_solute_hardening
                              ? &getMaterialPropertyOld<Real>("solute_concentration")
                              : nullptr),

    // Precipitate contributing populations
    _include_precipitate_hardening((parameters.isParamValid("precipitate_number_density") &&
                                    parameters.isParamValid("mean_precipitate_radius"))
                                       ? true
                                       : false),
    _precipitate_density(_include_precipitate_hardening
                             ? &getMaterialPropertyOld<Real>("precipitate_number_density")
                             : nullptr),
    _precipitate_radius(_include_precipitate_hardening
                            ? &getMaterialPropertyOld<Real>("mean_precipitate_radius")
                            : nullptr),

    // Plastic slip increment parameters
    _gamma_reference(getParam<Real>(_base_name + "reference_slip_rate")),
    _p_exp(getParam<Real>("strain_rate_sensitivity_exponent")),

    // plastic slip increment used in constitutive model calculations
    _coplanar_constitutive_slip_increment(
        declareProperty<std::vector<Real>>("coplanar_constitutive_slip_increment")),

    // Pinning point evolution calibration coefficients
    _self_pinpt_coeff(getParam<Real>(_base_name + "coefficient_self_plane_evolution")),
    _latent_pinpt_coeff(getParam<Real>(_base_name + "coefficient_latent_plane_evolution")),

    // Slip system resistance parameters
    _forest_hardening_coeff(
        getParam<Real>(_base_name + "forest_dislocation_hardening_coefficient")),
    _solute_hardening_coeff(getParam<Real>(_base_name + "solute_hardening_coefficient")),
    _precipitate_hardening_coeff(getParam<Real>(_base_name + "precipitate_hardening_coefficient")),
    _burgers_vector(getParam<Real>(_base_name + "burgers_vector")),
    _shear_modulus(getParam<Real>(_base_name + "shear_modulus")),

    // Twinning contributions, if used
    _include_twinning_in_Lp(parameters.isParamValid("total_twin_volume_fraction")),
    _twin_volume_fraction_total(_include_twinning_in_Lp
                                    ? &getMaterialPropertyOld<Real>("total_twin_volume_fraction")
                                    : nullptr)
{
  // resize local caching vectors used for substepping
  _previous_substep_slip_resistance.resize(_number_slip_systems);
  _previous_substep_pinning_points.resize(_number_coplanar_groups);
  _slip_resistance_before_update.resize(_number_slip_systems);
  _pinning_points_before_update.resize(_number_coplanar_groups);
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::initQpStatefulProperties()
{
  CrystalPlasticityStressUpdateBase::initQpStatefulProperties();
  // Resize constitutive-model specific material properties
  _pinning_point_density[_qp].resize(_number_coplanar_groups);
  _pinning_point_increment[_qp].resize(_number_coplanar_groups);
  _coplanar_constitutive_slip_increment[_qp].resize(_number_coplanar_groups);

  // Set constitutive-model specific initial values from parameters
  const Real pin_pts_density_per_plane = _initial_pinning_point_density / _number_coplanar_groups;
  for (const auto p : make_range(_number_coplanar_groups))
  {
    _pinning_point_density[_qp][p] = pin_pts_density_per_plane;
    _pinning_point_increment[_qp][p] = 0.0;
    _coplanar_constitutive_slip_increment[_qp][p] = 0.0;
  }

  for (const auto i : make_range(_number_slip_systems))
    _slip_increment[_qp][i] = 0.0;

  calculateInitialSlipResistance();
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::calculateInitialSlipResistance()
{
  std::vector<Real> solute_hardening(_number_coplanar_groups, 0.0);
  if (_include_solute_hardening)
    calculateSoluteResistance(solute_hardening);

  std::vector<Real> precipitate_hardening(_number_coplanar_groups, 0.0);
  if (_include_precipitate_hardening)
    calculatePrecipitateResistance(precipitate_hardening);

  for (const auto p : make_range(_number_coplanar_groups))
  {
    const Real hardening_sum = precipitate_hardening[p] + solute_hardening[p];

    for (const auto n : index_range(_coplanar_groups[p]))
      _slip_resistance[_qp][_coplanar_groups[p][n]] = hardening_sum;
  }
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::setInitialConstitutiveVariableValues()
{
  _slip_resistance[_qp] = _slip_resistance_old[_qp];
  _previous_substep_slip_resistance = _slip_resistance_old[_qp];

  _pinning_point_density[_qp] = _pinning_point_density_old[_qp];
  _previous_substep_pinning_points = _pinning_point_density_old[_qp];
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::setSubstepConstitutiveVariableValues()
{
  _slip_resistance[_qp] = _previous_substep_slip_resistance;
  _pinning_point_density[_qp] = _previous_substep_pinning_points;
}

bool
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::calculateSlipRate()
{
  bool allowable_slip_increment = true;

  for (const auto i : make_range(_number_slip_systems))
  {
    if (MooseUtils::absoluteFuzzyEqual(_tau[_qp][i], 0.0))
      _slip_increment[_qp][i] = 0.0;
    else
    {
      const Real driving_force = std::abs(_tau[_qp][i]) / _slip_resistance[_qp][i];
      _slip_increment[_qp][i] = _gamma_reference * std::pow(driving_force, (_p_exp));
      if (_tau[_qp][i] < 0.0)
        _slip_increment[_qp][i] *= -1.0;
    }

    // Check that none of the slip increments are outside of the allowable tolerance
    if (std::abs(_slip_increment[_qp][i] * _substep_dt) > _slip_incr_tol)
    {
      if (_print_convergence_message)
        mooseWarning("Maximum allowable slip increment exceeded ",
                     std::abs(_slip_increment[_qp][i] * _substep_dt));

      allowable_slip_increment = false;
    }
  }

  return allowable_slip_increment;
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::calculateEquivalentSlipIncrement(
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
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::calculateConstitutiveSlipDerivative(
    std::vector<Real> & dslip_dtau)
{
  for (const auto i : make_range(_number_slip_systems))
  {
    if (MooseUtils::absoluteFuzzyEqual(_tau[_qp][i], 0.0))
      dslip_dtau[i] = 0.0;
    else
      dslip_dtau[i] = _slip_increment[_qp][i] / (_p_exp * std::abs(_tau[_qp][i])) * _substep_dt;
  }
}

bool
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::areConstitutiveStateVariablesConverged()
{
  if (isConstitutiveStateVariableConverged(_pinning_point_density[_qp],
                                           _pinning_points_before_update,
                                           _previous_substep_pinning_points,
                                           _rel_state_var_tol) &&
      isConstitutiveStateVariableConverged(_slip_resistance[_qp],
                                           _slip_resistance_before_update,
                                           _previous_substep_slip_resistance,
                                           _resistance_tol))
    return true;
  return false;
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::updateSubstepConstitutiveVariableValues()
{
  _previous_substep_slip_resistance = _slip_resistance[_qp];
  _previous_substep_pinning_points = _pinning_point_density[_qp];
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::cacheStateVariablesBeforeUpdate()
{
  _slip_resistance_before_update = _slip_resistance[_qp];
  _pinning_points_before_update = _pinning_point_density[_qp];
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::calculateStateVariableEvolutionRateComponent()
{
  calculateConstitutiveCoplanarSlipIncrement();
  calculatePinningPointEvolutionIncrement();
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::calculateConstitutiveCoplanarSlipIncrement()
{
  // Compute the coplanar slip increment as the sum of the absolute slip increment
  // on each coplanar slip system
  for (const auto p : make_range(_number_coplanar_groups))
  {
    Real sum_coplanar_slip_increment = 0.0;
    for (const auto n : index_range(_coplanar_groups[p]))
      sum_coplanar_slip_increment += std::abs(_slip_increment[_qp][_coplanar_groups[p][n]]);

    _coplanar_constitutive_slip_increment[_qp][p] = sum_coplanar_slip_increment * _substep_dt;
  }
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::calculatePinningPointEvolutionIncrement()
{
  for (const auto p : make_range(_number_coplanar_groups))
  {
    Real increment = 0.0;
    for (const auto q : make_range(_number_coplanar_groups))
    {
      if (p == q) // self hardening
        increment += _self_pinpt_coeff * _coplanar_constitutive_slip_increment[_qp][q];
      else
        increment += _latent_pinpt_coeff * _coplanar_constitutive_slip_increment[_qp][q] /
                     (_number_coplanar_groups - 1.0);
    }
    _pinning_point_increment[_qp][p] = increment;
  }
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::calculateSlipResistance()
{
  std::vector<Real> forest_hardening(_number_coplanar_groups, 0.0);
  calculateForestSlipResistance(forest_hardening);

  std::vector<Real> solute_hardening(_number_coplanar_groups, 0.0);
  if (_include_solute_hardening)
    calculateSoluteResistance(solute_hardening);

  std::vector<Real> precipitate_hardening(_number_coplanar_groups, 0.0);
  if (_include_precipitate_hardening)
    calculatePrecipitateResistance(precipitate_hardening);

  for (const auto p : make_range(_number_coplanar_groups))
  {
    const Real sq_forest = Utility::pow<2>(forest_hardening[p]);
    const Real sq_precipitates = Utility::pow<2>(precipitate_hardening[p]);

    const Real hardening_sum = std::sqrt(sq_forest + sq_precipitates) + solute_hardening[p];
    for (const auto n : index_range(_coplanar_groups[p]))
      _slip_resistance[_qp][_coplanar_groups[p][n]] = hardening_sum;
  }
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::calculateForestSlipResistance(
    std::vector<Real> & forest_hardening)
{
  const Real lead_term = _forest_hardening_coeff * _shear_modulus * _burgers_vector;

  // by definition in this model, pinning points are mapped to a single plane
  // so we use a 1-to-1 calcuation of the hardening of each plane from the number
  // of pinning points associated with that plane
  for (const auto p : make_range(_number_coplanar_groups))
    forest_hardening[p] = lead_term * std::sqrt(_pinning_point_density[_qp][p]);
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::calculateSoluteResistance(
    std::vector<Real> & solute_hardening)
{
  const Real lead_term = _solute_hardening_coeff * _burgers_vector * _shear_modulus;
  const Real mean_free_path = _burgers_vector * (*_solute_concentration)[_qp];

  for (const auto j : make_range(_number_coplanar_groups))
    solute_hardening[j] = lead_term * std::sqrt(mean_free_path);
}

void
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::calculatePrecipitateResistance(
    std::vector<Real> & precipitate_hardening)
{
  const Real lead_term = _precipitate_hardening_coeff * _burgers_vector * _shear_modulus;
  const Real mean_free_path = 2.0 * (*_precipitate_density)[_qp] * (*_precipitate_radius)[_qp];

  for (const auto j : make_range(_number_coplanar_groups))
    precipitate_hardening[j] = lead_term * std::sqrt(mean_free_path);
}

bool
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::updateStateVariables()
{
  if (calculatePinningPointDensity())
    return true;
  else
    return false;
}

bool
CrystalPlasticityFCCDislocationLinkHuCocksUpdate::calculatePinningPointDensity()
{
  bool positive_pinning_point_density = true;
  for (const auto n : make_range(_number_coplanar_groups))
  {
    if (_previous_substep_pinning_points[n] < _zero_tol && _pinning_point_increment[_qp][n] < 0.0)
      _pinning_point_density[_qp][n] = _previous_substep_pinning_points[n];
    else
      _pinning_point_density[_qp][n] =
          _previous_substep_pinning_points[n] + _pinning_point_increment[_qp][n];

    if (_pinning_point_density[_qp][n] < 0.0)
    {
      mooseError("A negative pinning points density was computed");
      positive_pinning_point_density = false;
    }
  }
  return positive_pinning_point_density;
}
