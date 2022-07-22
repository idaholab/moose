//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemicalSystem.h"
#include "GeochemistryActivityCalculators.h"
#include "GeochemistryKineticRateCalculator.h"
#include "EquilibriumConstantInterpolator.h"

GeochemicalSystem::GeochemicalSystem(ModelGeochemicalDatabase & mgd,
                                     GeochemistryActivityCoefficients & gac,
                                     GeochemistryIonicStrength & is,
                                     GeochemistrySpeciesSwapper & swapper,
                                     const std::vector<std::string> & swap_out_of_basis,
                                     const std::vector<std::string> & swap_into_basis,
                                     const std::string & charge_balance_species,
                                     const std::vector<std::string> & constrained_species,
                                     const std::vector<Real> & constraint_value,
                                     const MultiMooseEnum & constraint_unit,
                                     const MultiMooseEnum & constraint_user_meaning,
                                     Real initial_temperature,
                                     unsigned iters_to_make_consistent,
                                     Real min_initial_molality,
                                     const std::vector<std::string> & kin_name,
                                     const std::vector<Real> & kin_initial,
                                     const MultiMooseEnum & kin_unit)
  : _mgd(mgd),
    _num_basis(mgd.basis_species_index.size()),
    _num_eqm(mgd.eqm_species_index.size()),
    _num_redox(mgd.redox_stoichiometry.m()),
    _num_surface_pot(mgd.surface_sorption_name.size()),
    _num_kin(mgd.kin_species_index.size()),
    _swapper(swapper),
    _swap_out(swap_out_of_basis),
    _swap_in(swap_into_basis),
    _gac(gac),
    _is(is),
    _charge_balance_species(charge_balance_species),
    _original_charge_balance_species(charge_balance_species),
    _charge_balance_basis_index(0),
    _constrained_species(constrained_species),
    _constraint_value(constraint_value),
    _original_constraint_value(constraint_value),
    _constraint_unit(constraint_unit.size()),
    _constraint_user_meaning(constraint_user_meaning.size()),
    _constraint_meaning(constraint_user_meaning.size()),
    _eqm_log10K(_num_eqm),
    _redox_log10K(_num_redox),
    _kin_log10K(_num_kin),
    _num_basis_in_algebraic_system(0),
    _num_in_algebraic_system(0),
    _in_algebraic_system(_num_basis),
    _algebraic_index(_num_basis),
    _basis_index(_num_basis),
    _bulk_moles_old(_num_basis),
    _basis_molality(_num_basis),
    _basis_activity_known(_num_basis),
    _basis_activity(_num_basis),
    _eqm_molality(_num_eqm),
    _basis_activity_coef(_num_basis),
    _eqm_activity_coef(_num_eqm),
    _eqm_activity(_num_eqm),
    _surface_pot_expr(_num_surface_pot),
    _sorbing_surface_area(_num_surface_pot),
    _kin_moles(_num_kin),
    _kin_moles_old(_num_kin),
    _iters_to_make_consistent(iters_to_make_consistent),
    _temperature(initial_temperature),
    _min_initial_molality(min_initial_molality),
    _original_redox_lhs()
{
  for (unsigned i = 0; i < constraint_user_meaning.size(); ++i)
    _constraint_user_meaning[i] =
        static_cast<ConstraintUserMeaningEnum>(constraint_user_meaning.get(i));
  for (unsigned i = 0; i < constraint_unit.size(); ++i)
    _constraint_unit[i] =
        static_cast<GeochemistryUnitConverter::GeochemistryUnit>(constraint_unit.get(i));
  const unsigned ku_size = kin_unit.size();
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> k_unit(ku_size);
  for (unsigned i = 0; i < ku_size; ++i)
    k_unit[i] = static_cast<GeochemistryUnitConverter::GeochemistryUnit>(kin_unit.get(i));
  checkAndInitialize(kin_name, kin_initial, k_unit);
}

GeochemicalSystem::GeochemicalSystem(
    ModelGeochemicalDatabase & mgd,
    GeochemistryActivityCoefficients & gac,
    GeochemistryIonicStrength & is,
    GeochemistrySpeciesSwapper & swapper,
    const std::vector<std::string> & swap_out_of_basis,
    const std::vector<std::string> & swap_into_basis,
    const std::string & charge_balance_species,
    const std::vector<std::string> & constrained_species,
    const std::vector<Real> & constraint_value,
    const std::vector<GeochemistryUnitConverter::GeochemistryUnit> & constraint_unit,
    const std::vector<ConstraintUserMeaningEnum> & constraint_user_meaning,
    Real initial_temperature,
    unsigned iters_to_make_consistent,
    Real min_initial_molality,
    const std::vector<std::string> & kin_name,
    const std::vector<Real> & kin_initial,
    const std::vector<GeochemistryUnitConverter::GeochemistryUnit> & kin_unit)
  : _mgd(mgd),
    _num_basis(mgd.basis_species_index.size()),
    _num_eqm(mgd.eqm_species_index.size()),
    _num_redox(mgd.redox_stoichiometry.m()),
    _num_surface_pot(mgd.surface_sorption_name.size()),
    _num_kin(mgd.kin_species_index.size()),
    _swapper(swapper),
    _swap_out(swap_out_of_basis),
    _swap_in(swap_into_basis),
    _gac(gac),
    _is(is),
    _charge_balance_species(charge_balance_species),
    _original_charge_balance_species(charge_balance_species),
    _charge_balance_basis_index(0),
    _constrained_species(constrained_species),
    _constraint_value(constraint_value),
    _original_constraint_value(constraint_value),
    _constraint_unit(constraint_unit),
    _constraint_user_meaning(constraint_user_meaning),
    _constraint_meaning(constraint_user_meaning.size()),
    _eqm_log10K(_num_eqm),
    _redox_log10K(_num_redox),
    _kin_log10K(_num_kin),
    _num_basis_in_algebraic_system(0),
    _num_in_algebraic_system(0),
    _in_algebraic_system(_num_basis),
    _algebraic_index(_num_basis),
    _basis_index(_num_basis),
    _bulk_moles_old(_num_basis),
    _basis_molality(_num_basis),
    _basis_activity_known(_num_basis),
    _basis_activity(_num_basis),
    _eqm_molality(_num_eqm),
    _basis_activity_coef(_num_basis),
    _eqm_activity_coef(_num_eqm),
    _eqm_activity(_num_eqm),
    _surface_pot_expr(_num_surface_pot),
    _sorbing_surface_area(_num_surface_pot),
    _kin_moles(_num_kin),
    _kin_moles_old(_num_kin),
    _iters_to_make_consistent(iters_to_make_consistent),
    _temperature(initial_temperature),
    _min_initial_molality(min_initial_molality),
    _original_redox_lhs()
{
  checkAndInitialize(kin_name, kin_initial, kin_unit);
}

void
GeochemicalSystem::checkAndInitialize(
    const std::vector<std::string> & kin_name,
    const std::vector<Real> & kin_initial,
    const std::vector<GeochemistryUnitConverter::GeochemistryUnit> & kin_unit)
{
  // initialize every the kinetic species
  const unsigned num_kin_name = kin_name.size();
  if (!(_num_kin == num_kin_name && _num_kin == kin_initial.size() && _num_kin == kin_unit.size()))
    mooseError("Initial mole number (or mass or volume) and a unit must be provided for each "
               "kinetic species ",
               _num_kin,
               " ",
               num_kin_name,
               " ",
               kin_initial.size(),
               " ",
               kin_unit.size());
  for (const auto & name_index : _mgd.kin_species_index)
  {
    const unsigned ind = std::distance(
        kin_name.begin(), std::find(kin_name.begin(), kin_name.end(), name_index.first));
    if (ind < num_kin_name)
    {
      if (!(kin_unit[ind] == GeochemistryUnitConverter::GeochemistryUnit::MOLES ||
            kin_unit[ind] == GeochemistryUnitConverter::GeochemistryUnit::KG ||
            kin_unit[ind] == GeochemistryUnitConverter::GeochemistryUnit::G ||
            kin_unit[ind] == GeochemistryUnitConverter::GeochemistryUnit::MG ||
            kin_unit[ind] == GeochemistryUnitConverter::GeochemistryUnit::UG ||
            kin_unit[ind] == GeochemistryUnitConverter::GeochemistryUnit::CM3))
        mooseError("Kinetic species ",
                   name_index.first,
                   ": units must be moles or mass, or volume in the case of minerals");
      const Real moles = GeochemistryUnitConverter::toMoles(
          kin_initial[ind], kin_unit[ind], name_index.first, _mgd);
      setKineticMoles(name_index.second, moles);
    }
    else
      mooseError("Initial mole number or mass or volume for kinetic species ",
                 name_index.first,
                 " must be provided");
  }

  // check sanity of swaps
  if (_swap_out.size() != _swap_in.size())
    mooseError("swap_out_of_basis must have same length as swap_into_basis");
  for (unsigned i = 0; i < _swap_out.size(); ++i)
    if (_swap_out[i] == _charge_balance_species)
      mooseError("Cannot swap out ",
                 _charge_balance_species,
                 " because it is the charge-balance species\n");

  // do swaps desired by user.  any exception here is an error
  for (unsigned i = 0; i < _swap_out.size(); ++i)
    try
    {
      _swapper.performSwap(_mgd, _swap_out[i], _swap_in[i]);
    }
    catch (const MooseException & e)
    {
      mooseError(e.what());
    }

  // check charge-balance species is in the basis and has a charge
  if (_mgd.basis_species_index.count(_charge_balance_species) == 0)
    mooseError("Cannot enforce charge balance using ",
               _charge_balance_species,
               " because it is not in the basis");
  _charge_balance_basis_index = _mgd.basis_species_index.at(_charge_balance_species);
  if (_mgd.basis_species_charge[_charge_balance_basis_index] == 0.0)
    mooseError("Cannot enforce charge balance using ",
               _charge_balance_species,
               " because it has zero charge");

  // check that constraint vectors have appropriate sizes
  if (_constrained_species.size() != _constraint_value.size())
    mooseError("Constrained species names must have same length as constraint values");
  if (_constrained_species.size() != _constraint_unit.size())
    mooseError("Constrained species names must have same length as constraint units");
  if (_constrained_species.size() != _constraint_user_meaning.size())
    mooseError("Constrained species names must have same length as constraint meanings");
  if (_constrained_species.size() != _num_basis)
    mooseError("Constrained species names must have same length as the number of species in the "
               "basis (each component must be provided with a single constraint");

  // check that each _constrained_species name appears in the basis
  for (const auto & name : _mgd.basis_species_name)
    if (std::find(_constrained_species.begin(), _constrained_species.end(), name) ==
        _constrained_species.end())
      mooseError("The basis species ", name, " must appear in the constrained species list");

  // order the constraints in the same way as the basis species.  This makes the remainder of the
  // code much cleaner.
  std::vector<std::string> c_s(_num_basis);
  std::vector<Real> c_v(_num_basis);
  std::vector<GeochemistryUnitConverter::GeochemistryUnit> c_u(_num_basis);
  std::vector<ConstraintUserMeaningEnum> c_m(_num_basis);
  for (unsigned i = 0; i < _num_basis; ++i)
  {
    const unsigned basis_ind = _mgd.basis_species_index.at(_constrained_species[i]);
    c_s[basis_ind] = _constrained_species[i];
    c_v[basis_ind] = _constraint_value[i];
    c_u[basis_ind] = _constraint_unit[i];
    c_m[basis_ind] = _constraint_user_meaning[i];
  }
  _constrained_species = c_s;
  _constraint_value = c_v;
  _constraint_unit = c_u;
  _original_constraint_value = c_v;
  _constraint_user_meaning = c_m;

  // run through the constraints, checking physical and chemical consistency, converting to mole
  // units, and building constraint_meaning
  for (unsigned i = 0; i < _constrained_species.size(); ++i)
  {
    const std::string name = _constrained_species[i];

    switch (_constraint_user_meaning[i])
    {
      case ConstraintUserMeaningEnum::KG_SOLVENT_WATER:
      {
        // if the mass of solvent water is provided, check it is positive
        if (_constraint_value[i] <= 0.0)
          mooseError("Specified mass of solvent water must be positive: you entered ",
                     _constraint_value[i]);
        if (_constraint_unit[i] != GeochemistryUnitConverter::GeochemistryUnit::KG)
          mooseError("Units for kg_solvent_water must be kg");
        _constraint_meaning[i] = ConstraintMeaningEnum::KG_SOLVENT_WATER;
        break;
      }
      case ConstraintUserMeaningEnum::BULK_COMPOSITION:
      case ConstraintUserMeaningEnum::BULK_COMPOSITION_WITH_KINETIC:
      {
        // convert to mole units and specify correct constraint_meaning
        _constraint_value[i] = GeochemistryUnitConverter::toMoles(
            _constraint_value[i], _constraint_unit[i], name, _mgd);
        // add contributions from kinetic moles, if necessary
        if (_constraint_user_meaning[i] == ConstraintUserMeaningEnum::BULK_COMPOSITION)
          for (unsigned k = 0; k < _mgd.kin_species_name.size(); ++k)
            _constraint_value[i] += _kin_moles[k] * _mgd.kin_stoichiometry(k, i);
        if (!(_constraint_unit[i] == GeochemistryUnitConverter::GeochemistryUnit::MOLES ||
              _constraint_unit[i] == GeochemistryUnitConverter::GeochemistryUnit::KG ||
              _constraint_unit[i] == GeochemistryUnitConverter::GeochemistryUnit::G ||
              _constraint_unit[i] == GeochemistryUnitConverter::GeochemistryUnit::MG ||
              _constraint_unit[i] == GeochemistryUnitConverter::GeochemistryUnit::UG))
          mooseError("Species ", name, ": units for bulk composition must be moles or mass");
        if (name == "H2O")
          _constraint_meaning[i] = ConstraintMeaningEnum::MOLES_BULK_WATER;
        else
          _constraint_meaning[i] = ConstraintMeaningEnum::MOLES_BULK_SPECIES;
        break;
      }
      case ConstraintUserMeaningEnum::FREE_CONCENTRATION:
      {
        // if free concentration, check it is positive and perform the translation to mole units
        if (_constraint_value[i] <= 0.0)
          mooseError("Specified free concentration values must be positive: you entered ",
                     _constraint_value[i]);
        if (!(_constraint_unit[i] == GeochemistryUnitConverter::GeochemistryUnit::MOLAL ||
              _constraint_unit[i] ==
                  GeochemistryUnitConverter::GeochemistryUnit::KG_PER_KG_SOLVENT ||
              _constraint_unit[i] ==
                  GeochemistryUnitConverter::GeochemistryUnit::G_PER_KG_SOLVENT ||
              _constraint_unit[i] ==
                  GeochemistryUnitConverter::GeochemistryUnit::MG_PER_KG_SOLVENT ||
              _constraint_unit[i] ==
                  GeochemistryUnitConverter::GeochemistryUnit::UG_PER_KG_SOLVENT))
          mooseError(
              "Species ",
              name,
              ": units for free concentration quantities must be molal or mass_per_kg_solvent");
        _constraint_value[i] = GeochemistryUnitConverter::toMoles(
            _constraint_value[i], _constraint_unit[i], name, _mgd);
        _constraint_meaning[i] = ConstraintMeaningEnum::FREE_MOLALITY;
        break;
      }
      case ConstraintUserMeaningEnum::FREE_MINERAL:
      {
        // if free mineral, check it is positive and perform the translation to mole units
        if (_constraint_value[i] <= 0.0)
          mooseError("Specified free mineral values must be positive: you entered ",
                     _constraint_value[i]);
        if (!(_constraint_unit[i] == GeochemistryUnitConverter::GeochemistryUnit::MOLES ||
              _constraint_unit[i] == GeochemistryUnitConverter::GeochemistryUnit::KG ||
              _constraint_unit[i] == GeochemistryUnitConverter::GeochemistryUnit::G ||
              _constraint_unit[i] == GeochemistryUnitConverter::GeochemistryUnit::MG ||
              _constraint_unit[i] == GeochemistryUnitConverter::GeochemistryUnit::UG ||
              _constraint_unit[i] == GeochemistryUnitConverter::GeochemistryUnit::CM3))
          mooseError("Species ",
                     name,
                     ": units for free mineral quantities must be moles, mass or volume");
        _constraint_value[i] = GeochemistryUnitConverter::toMoles(
            _constraint_value[i], _constraint_unit[i], name, _mgd);
        _constraint_meaning[i] = ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES;
        break;
      }
      case ConstraintUserMeaningEnum::ACTIVITY:
      {
        if (_constraint_value[i] <= 0.0)
          mooseError("Specified activity values must be positive: you entered ",
                     _constraint_value[i]);
        if (_constraint_unit[i] != GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS)
          mooseError(
              "Species ", name, ": dimensionless units must be used when specifying activity");
        _constraint_meaning[i] = ConstraintMeaningEnum::ACTIVITY;
        break;
      }
      case ConstraintUserMeaningEnum::LOG10ACTIVITY:
      {
        // if log10activity is provided, convert it to activity
        if (_constraint_unit[i] != GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS)
          mooseError("Species ",
                     name,
                     ": dimensionless units must be used when specifying log10activity\n");
        _constraint_value[i] = std::pow(10.0, _constraint_value[i]);
        _constraint_meaning[i] = ConstraintMeaningEnum::ACTIVITY;
        break;
      }
      case ConstraintUserMeaningEnum::FUGACITY:
      {
        // if fugacity is provided, check it is positive
        if (_constraint_value[i] <= 0.0)
          mooseError("Specified fugacity values must be positive: you entered ",
                     _constraint_value[i]);
        if (_constraint_unit[i] != GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS)
          mooseError(
              "Species ", name, ": dimensionless units must be used when specifying fugacity\n");
        _constraint_meaning[i] = ConstraintMeaningEnum::FUGACITY;
        break;
      }
      case ConstraintUserMeaningEnum::LOG10FUGACITY:
      {
        // if log10fugacity is provided, convert it to fugacity
        if (_constraint_unit[i] != GeochemistryUnitConverter::GeochemistryUnit::DIMENSIONLESS)
          mooseError("Species ",
                     name,
                     ": dimensionless units must be used when specifying log10fugacity\n");
        _constraint_value[i] = std::pow(10.0, _constraint_value[i]);
        _constraint_meaning[i] = ConstraintMeaningEnum::FUGACITY;
        break;
      }
    }

    // check that water is provided with correct meaning
    if (name == "H2O")
      if (!(_constraint_meaning[i] == ConstraintMeaningEnum::MOLES_BULK_WATER ||
            _constraint_meaning[i] == ConstraintMeaningEnum::KG_SOLVENT_WATER ||
            _constraint_meaning[i] == ConstraintMeaningEnum::ACTIVITY))
        mooseError("H2O must be provided with either a mass of solvent water, a bulk composition "
                   "in moles or mass, or an activity");

    // check that gases are provided with the correct meaning
    if (_mgd.basis_species_gas[i])
      if (_constraint_meaning[i] != ConstraintMeaningEnum::FUGACITY)
        mooseError("The gas ", name, " must be provided with a fugacity");

    // check that minerals are provided with the correct meaning
    if (_mgd.basis_species_mineral[i])
      if (!(_constraint_meaning[i] == ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES ||
            _constraint_meaning[i] == ConstraintMeaningEnum::MOLES_BULK_SPECIES))
        mooseError("The mineral ",
                   name,
                   " must be provided with either: a free number of moles, a free mass or a free "
                   "volume; or a bulk composition of moles or mass");

    // check that non-water, non-minerals, non-gases are provided with the correct meaning
    if (name != "H2O" && !_mgd.basis_species_gas[i] && !_mgd.basis_species_mineral[i])
      if (!(_constraint_meaning[i] == ConstraintMeaningEnum::FREE_MOLALITY ||
            _constraint_meaning[i] == ConstraintMeaningEnum::ACTIVITY ||
            _constraint_meaning[i] == ConstraintMeaningEnum::MOLES_BULK_SPECIES))
        mooseError("The basis species ",
                   name,
                   " must be provided with a free concentration, bulk composition or an activity");

    // check that the charge-balance species has been provided MOLES_BULK_SPECIES
    if (name == _charge_balance_species)
      if (_constraint_meaning[i] != ConstraintMeaningEnum::MOLES_BULK_SPECIES)
        mooseError("For code consistency, the species ",
                   name,
                   " must be provided with a bulk composition because it is the charge-balance "
                   "species.  The value provided should be a reasonable estimate of the mole "
                   "number, but will be overridden as the solve progresses");
  }
  _original_redox_lhs = _mgd.redox_lhs;

  initialize();
}

void
GeochemicalSystem::initialize()
{
  buildTemperatureDependentQuantities(_temperature);
  enforceChargeBalanceIfSimple(_constraint_value, _bulk_moles_old);
  buildAlgebraicInfo(_in_algebraic_system,
                     _num_basis_in_algebraic_system,
                     _num_in_algebraic_system,
                     _algebraic_index,
                     _basis_index);
  initBulkAndFree(_bulk_moles_old, _basis_molality);
  buildKnownBasisActivities(_basis_activity_known, _basis_activity);

  _eqm_molality.assign(_num_eqm, 0.0);
  _surface_pot_expr.assign(_num_surface_pot, 1.0);

  computeConsistentConfiguration();
}

void
GeochemicalSystem::computeConsistentConfiguration()
{
  // the steps 1 and 2 below could be iterated for a long time (or a Newton process could even be
  // followed) to provide better estimates of activities and molalities, but this is not done in the
  // conventional geochemistry approach: there are just too many unknowns and approximations
  // employed during the algebraic-system solve to justify iterating towards the perfectly
  // consistent initial condition
  for (unsigned picard = 0; picard < _iters_to_make_consistent + 1; ++picard)
  {
    // Step 1: compute ionic strengths and activities using the eqm molalities
    _gac.setInternalParameters(_temperature, _mgd, _basis_molality, _eqm_molality, _kin_moles);
    _gac.buildActivityCoefficients(_mgd, _basis_activity_coef, _eqm_activity_coef);
    updateBasisMolalityForKnownActivity(_basis_molality);
    computeRemainingBasisActivities(_basis_activity);

    // Step 2: compute equilibrium molality based on the activities just computed
    computeEqmMolalities(_eqm_molality);
  }

  computeBulk(_bulk_moles_old);
  computeFreeMineralMoles(_basis_molality);
  computeSorbingSurfaceArea(_sorbing_surface_area);
}

unsigned
GeochemicalSystem::getChargeBalanceBasisIndex() const
{
  return _charge_balance_basis_index;
}

Real
GeochemicalSystem::getLog10K(unsigned j) const
{
  if (j >= _num_eqm)
    mooseError("Cannot retrieve log10K for equilibrium species ",
               j,
               " since there are only ",
               _num_eqm,
               " equilibrium species");
  return _eqm_log10K[j];
}

unsigned
GeochemicalSystem::getNumRedox() const
{
  return _num_redox;
}

Real
GeochemicalSystem::getRedoxLog10K(unsigned red) const
{
  if (red >= _num_redox)
    mooseError("Cannot retrieve log10K for redox species ",
               red,
               " since there are only ",
               _num_redox,
               " redox species");
  return _redox_log10K[red];
}

Real
GeochemicalSystem::log10RedoxActivityProduct(unsigned red) const
{
  if (red >= _num_redox)
    mooseError("Cannot retrieve activity product for redox species ",
               red,
               " since there are only ",
               _num_redox,
               " redox species");
  Real log10ap = 0.0;
  for (unsigned basis_i = 0; basis_i < _num_basis; ++basis_i)
    log10ap += _mgd.redox_stoichiometry(red, basis_i) * std::log10(_basis_activity[basis_i]);
  return log10ap;
}

unsigned
GeochemicalSystem::getNumKinetic() const
{
  return _num_kin;
}

Real
GeochemicalSystem::getKineticLog10K(unsigned kin) const
{
  if (kin >= _num_kin)
    mooseError("Cannot retrieve log10K for kinetic species ",
               kin,
               " since there are only ",
               _num_kin,
               " kinetic species");
  return _kin_log10K[kin];
}

const std::vector<Real> &
GeochemicalSystem::getKineticLog10K() const
{
  return _kin_log10K;
}

Real
GeochemicalSystem::log10KineticActivityProduct(unsigned kin) const
{
  if (kin >= _num_kin)
    mooseError("Cannot retrieve activity product for kinetic species ",
               kin,
               " since there are only ",
               _num_kin,
               " kinetic species");
  Real log10ap = 0.0;
  for (unsigned basis_i = 0; basis_i < _num_basis; ++basis_i)
    log10ap += _mgd.kin_stoichiometry(kin, basis_i) * std::log10(_basis_activity[basis_i]);
  return log10ap;
}

void
GeochemicalSystem::buildTemperatureDependentQuantities(Real temperature)
{
  const std::vector<Real> temps = _mgd.original_database->getTemperatures();
  const unsigned numT = temps.size();
  const std::string model_type = _mgd.original_database->getLogKModel();

  for (unsigned eqm_j = 0; eqm_j < _num_eqm; ++eqm_j)
  {
    EquilibriumConstantInterpolator interp(
        temps, _mgd.eqm_log10K.sub_matrix(eqm_j, 1, 0, numT).get_values(), model_type);
    interp.generate();
    _eqm_log10K[eqm_j] = interp.sample(temperature);
  }
  for (unsigned red = 0; red < _num_redox; ++red)
  {
    EquilibriumConstantInterpolator interp(
        temps, _mgd.redox_log10K.sub_matrix(red, 1, 0, numT).get_values(), model_type);
    interp.generate();
    _redox_log10K[red] = interp.sample(temperature);
  }
  for (unsigned kin = 0; kin < _num_kin; ++kin)
  {
    EquilibriumConstantInterpolator interp(
        temps, _mgd.kin_log10K.sub_matrix(kin, 1, 0, numT).get_values(), model_type);
    interp.generate();
    _kin_log10K[kin] = interp.sample(temperature);
  }
}

void
GeochemicalSystem::buildAlgebraicInfo(std::vector<bool> & in_algebraic_system,
                                      unsigned & num_basis_in_algebraic_system,
                                      unsigned & num_in_algebraic_system,
                                      std::vector<unsigned> & algebraic_index,
                                      std::vector<unsigned> & basis_index) const
{
  // build in_algebraic_system
  for (const auto & name_index : _mgd.basis_species_index)
  {
    const std::string name = name_index.first;
    const unsigned basis_ind = name_index.second;
    const ConstraintMeaningEnum meaning = _constraint_meaning[basis_ind];
    if (name == "H2O")
      in_algebraic_system[basis_ind] = (meaning == ConstraintMeaningEnum::MOLES_BULK_WATER);
    else if (_mgd.basis_species_gas[basis_ind])
      in_algebraic_system[basis_ind] = false;
    else if (_mgd.basis_species_mineral[basis_ind])
      in_algebraic_system[basis_ind] = false;
    else
      in_algebraic_system[basis_ind] = (meaning == ConstraintMeaningEnum::MOLES_BULK_SPECIES);
  }

  // build algebraic_index and basis_index
  num_basis_in_algebraic_system = 0;
  algebraic_index.resize(_num_basis, 0);
  basis_index.resize(_num_basis, 0);
  for (unsigned basis_ind = 0; basis_ind < _num_basis; ++basis_ind)
    if (in_algebraic_system[basis_ind])
    {
      algebraic_index[basis_ind] = _num_basis_in_algebraic_system;
      basis_index[_num_basis_in_algebraic_system] = basis_ind;
      num_basis_in_algebraic_system += 1;
    }

  num_in_algebraic_system = num_basis_in_algebraic_system + _num_surface_pot + _num_kin;
}

unsigned
GeochemicalSystem::getNumInAlgebraicSystem() const
{
  return _num_in_algebraic_system;
}

unsigned
GeochemicalSystem::getNumBasisInAlgebraicSystem() const
{
  return _num_basis_in_algebraic_system;
}

unsigned
GeochemicalSystem::getNumSurfacePotentials() const
{
  return _num_surface_pot;
}

const std::vector<bool> &
GeochemicalSystem::getInAlgebraicSystem() const
{
  return _in_algebraic_system;
}

const std::vector<unsigned> &
GeochemicalSystem::getBasisIndexOfAlgebraicSystem() const
{
  return _basis_index;
}

const std::vector<unsigned> &
GeochemicalSystem::getAlgebraicIndexOfBasisSystem() const
{
  return _algebraic_index;
}

std::vector<Real>
GeochemicalSystem::getAlgebraicVariableValues() const
{
  std::vector<Real> var(_num_basis_in_algebraic_system + _num_surface_pot + _num_kin);
  for (unsigned a = 0; a < _num_basis_in_algebraic_system; ++a)
    var[a] = _basis_molality[_basis_index[a]];
  for (unsigned s = 0; s < _num_surface_pot; ++s)
    var[s + _num_basis_in_algebraic_system] = _surface_pot_expr[s];
  for (unsigned k = 0; k < _num_kin; ++k)
    var[k + _num_basis_in_algebraic_system + _num_surface_pot] = _kin_moles[k];
  return var;
}

std::vector<Real>
GeochemicalSystem::getAlgebraicBasisValues() const
{
  std::vector<Real> var(_num_basis_in_algebraic_system);
  for (unsigned a = 0; a < _num_basis_in_algebraic_system; ++a)
    var[a] = _basis_molality[_basis_index[a]];
  return var;
}

DenseVector<Real>
GeochemicalSystem::getAlgebraicVariableDenseValues() const
{
  DenseVector<Real> var(_num_in_algebraic_system);
  for (unsigned a = 0; a < _num_basis_in_algebraic_system; ++a)
    var(a) = _basis_molality[_basis_index[a]];
  for (unsigned s = 0; s < _num_surface_pot; ++s)
    var(s + _num_basis_in_algebraic_system) = _surface_pot_expr[s];
  for (unsigned k = 0; k < _num_kin; ++k)
    var(k + _num_basis_in_algebraic_system + _num_surface_pot) = _kin_moles[k];
  return var;
}

void
GeochemicalSystem::initBulkAndFree(std::vector<Real> & bulk_moles_old,
                                   std::vector<Real> & basis_molality) const
{
  for (unsigned i = 0; i < _num_basis; ++i)
  {
    // water is done first, so dividing by basis_molality[0] is OK
    const Real value = _constraint_value[i];
    const ConstraintMeaningEnum meaning = _constraint_meaning[i];
    switch (meaning)
    {
      case ConstraintMeaningEnum::MOLES_BULK_WATER:
      {
        bulk_moles_old[i] = value;
        basis_molality[i] = std::max(
            _min_initial_molality,
            0.999 * value /
                GeochemistryConstants::MOLES_PER_KG_WATER); // mass of solvent water (water is an
                                                            // algebraic variable). Guess used to
                                                            // initialize the Newton process
        break;
      }
      case ConstraintMeaningEnum::KG_SOLVENT_WATER:
      {
        bulk_moles_old[i] = value * GeochemistryConstants::MOLES_PER_KG_WATER /
                            0.999; // initial guess (water is not an algebraic variable).  Will be
                                   // determined exactly during the solve
        basis_molality[i] = value; // mass of solvent water
        break;
      }
      case ConstraintMeaningEnum::MOLES_BULK_SPECIES:
      {
        bulk_moles_old[i] = value;
        basis_molality[i] = std::max(
            _min_initial_molality,
            0.9 * value / basis_molality[0]); // initial guess (i is an algebraic variable).  This
                                              // is what we solve for in the Newton process
        break;
      }
      case ConstraintMeaningEnum::FREE_MOLALITY:
      {
        bulk_moles_old[i] =
            value * basis_molality[0] / 0.9; // initial guess (i is not an algebraic variable).
                                             // Will be determined exactly during the solve
        basis_molality[i] = value;
        break;
      }
      case ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES:
      {
        bulk_moles_old[i] = value / 0.9; // initial guess (i is not an algebraic variable).  Will
                                         // be determined exactly during the solve
        basis_molality[i] = value;       // note, this is *moles*, not molality
        break;
      }
      case ConstraintMeaningEnum::FUGACITY:
      {
        bulk_moles_old[i] = 0.0; // initial guess (i is not an algebraic variable).  will be
                                 // determined exactly after the solve
        basis_molality[i] =
            0.0; // never used in any solve process, but since this is a gas should be zero, and
                 // setting this explicitly elimiates if(species=gas) checks in various loops
        break;
      }
      case ConstraintMeaningEnum::ACTIVITY:
      {
        bulk_moles_old[i] = value / 0.9; // initial guess (i is not an algebraic variable).  Will
                                         // be determined exactly during the solve
        if (i == 0)
          basis_molality[i] = 1.0; // assumption
        else
          basis_molality[i] =
              value /
              0.9; // assume activity_coefficient = 0.9.  this will be updated during the solve
        break;
      }
    }
  }
}

Real
GeochemicalSystem::getSolventWaterMass() const
{
  return _basis_molality[0];
}

const std::vector<Real> &
GeochemicalSystem::getBulkMolesOld() const
{
  return _bulk_moles_old;
}

DenseVector<Real>
GeochemicalSystem::getBulkOldInOriginalBasis() const
{
  DenseVector<Real> result(_bulk_moles_old);
  if (_mgd.swap_to_original_basis.n() == 0)
    return result; // no swaps have been performed
  _mgd.swap_to_original_basis.vector_mult_transpose(result, _bulk_moles_old);
  return result;
}

DenseVector<Real>
GeochemicalSystem::getTransportedBulkInOriginalBasis() const
{
  std::vector<Real> trans_bulk;
  computeTransportedBulkFromMolalities(trans_bulk);
  DenseVector<Real> result(trans_bulk);
  if (_mgd.swap_to_original_basis.n() == 0)
    return result; // no swaps have been performed
  _mgd.swap_to_original_basis.vector_mult_transpose(result, trans_bulk);
  return result;
}

const std::vector<Real> &
GeochemicalSystem::getSolventMassAndFreeMolalityAndMineralMoles() const
{
  return _basis_molality;
}

void
GeochemicalSystem::buildKnownBasisActivities(std::vector<bool> & basis_activity_known,
                                             std::vector<Real> & basis_activity) const
{
  basis_activity_known = std::vector<bool>(_num_basis, false);
  basis_activity.resize(_num_basis);

  // all aqueous species with provided activity, and all gases with fugacity have known activity
  for (unsigned i = 0; i < _num_basis; ++i)
  {
    const ConstraintMeaningEnum meaning = _constraint_meaning[i];
    if (meaning == ConstraintMeaningEnum::ACTIVITY || meaning == ConstraintMeaningEnum::FUGACITY)
    {
      basis_activity_known[i] = true;
      basis_activity[i] = _constraint_value[i];
    }
  }

  // all minerals have activity = 1.0
  for (unsigned basis_ind = 0; basis_ind < _num_basis; ++basis_ind)
    if (_mgd.basis_species_mineral[basis_ind])
    {
      basis_activity_known[basis_ind] = true;
      basis_activity[basis_ind] = 1.0;
    }
}

const std::vector<bool> &
GeochemicalSystem::getBasisActivityKnown() const
{
  return _basis_activity_known;
}

Real
GeochemicalSystem::getBasisActivity(unsigned i) const
{
  if (i >= _num_basis)
    mooseError("Cannot retrieve basis activity for species ",
               i,
               " since there are only ",
               _num_basis,
               " basis species");
  return _basis_activity[i];
}

const std::vector<Real> &
GeochemicalSystem::getBasisActivity() const
{
  return _basis_activity;
}

Real
GeochemicalSystem::getEquilibriumMolality(unsigned j) const
{
  if (j >= _num_eqm)
    mooseError("Cannot retrieve molality for equilibrium species ",
               j,
               " since there are only ",
               _num_eqm,
               " equilibrium species");
  return _eqm_molality[j];
}

const std::vector<Real> &
GeochemicalSystem::getEquilibriumMolality() const
{
  return _eqm_molality;
}

Real
GeochemicalSystem::getKineticMoles(unsigned k) const
{
  if (k >= _num_kin)
    mooseError("Cannot retrieve moles for kinetic species ",
               k,
               " since there are only ",
               _num_kin,
               " kinetic species");
  return _kin_moles[k];
}

void
GeochemicalSystem::setKineticMoles(unsigned k, Real moles)
{
  if (k >= _num_kin)
    mooseError("Cannot set moles for kinetic species ",
               k,
               " since there are only ",
               _num_kin,
               " kinetic species");
  if (moles <= 0.0)
    mooseError("Mole number for kinetic species must be positive, not ", moles);
  _kin_moles[k] = moles;
  _kin_moles_old[k] = moles;
}

const std::vector<Real> &
GeochemicalSystem::getKineticMoles() const
{
  return _kin_moles;
}

void
GeochemicalSystem::computeRemainingBasisActivities(std::vector<Real> & basis_activity) const
{
  if (!_basis_activity_known[0])
    basis_activity[0] = _gac.waterActivity();
  for (unsigned basis_ind = 1; basis_ind < _num_basis; ++basis_ind) // don't loop over water
    if (!_basis_activity_known[basis_ind]) // basis_activity_known = true for minerals, gases and
                                           // species with activities provided by the user
      basis_activity[basis_ind] = _basis_activity_coef[basis_ind] * _basis_molality[basis_ind];
}

Real
GeochemicalSystem::getEquilibriumActivityCoefficient(unsigned j) const
{
  if (j >= _num_eqm)
    mooseError("Cannot retrieve activity coefficient for equilibrium species ",
               j,
               " since there are only ",
               _num_eqm,
               " equilibrium species");
  return _eqm_activity_coef[j];
}

const std::vector<Real> &
GeochemicalSystem::getEquilibriumActivityCoefficient() const
{
  return _eqm_activity_coef;
}

Real
GeochemicalSystem::getBasisActivityCoefficient(unsigned i) const
{
  if (i >= _num_basis)
    mooseError("Cannot retrieve basis activity coefficient for species ",
               i,
               " since there are only ",
               _num_basis,
               " basis species");
  return _basis_activity_coef[i];
}

const std::vector<Real> &
GeochemicalSystem::getBasisActivityCoefficient() const
{
  return _basis_activity_coef;
}

void
GeochemicalSystem::updateBasisMolalityForKnownActivity(std::vector<Real> & basis_molality) const
{
  for (unsigned i = 1; i < _num_basis; ++i) // don't loop over water
  {
    if (_basis_activity_known[i] && !_mgd.basis_species_mineral[i] && !_mgd.basis_species_gas[i])
      basis_molality[i] =
          _basis_activity[i] / _basis_activity_coef[i]; // just those for
                                                        // which activity is provided by the user
  }
}

Real
GeochemicalSystem::log10ActivityProduct(unsigned eqm_j) const
{
  Real log10ap = 0.0;
  for (unsigned basis_i = 0; basis_i < _num_basis; ++basis_i)
    log10ap += _mgd.eqm_stoichiometry(eqm_j, basis_i) * std::log10(_basis_activity[basis_i]);
  return log10ap;
}

void
GeochemicalSystem::computeEqmMolalities(std::vector<Real> & eqm_molality) const
{
  for (unsigned eqm_j = 0; eqm_j < _num_eqm; ++eqm_j)
  {
    if (_mgd.eqm_species_mineral[eqm_j] || _mgd.eqm_species_gas[eqm_j])
      eqm_molality[eqm_j] = 0.0;
    else
    {
      // compute log10 version first, in an attempt to eliminate overflow and underflow problems
      // such as 10^(1000)
      const Real log10m = log10ActivityProduct(eqm_j) - _eqm_log10K[eqm_j];
      eqm_molality[eqm_j] =
          std::pow(10.0, log10m) / _eqm_activity_coef[eqm_j] * surfaceSorptionModifier(eqm_j);
    }
  }
}

Real
GeochemicalSystem::surfaceSorptionModifier(unsigned eqm_j) const
{
  if (eqm_j >= _num_eqm)
    return 1.0;
  if (!_mgd.surface_sorption_related[eqm_j])
    return 1.0;
  return std::pow(_surface_pot_expr[_mgd.surface_sorption_number[eqm_j]],
                  2.0 * _mgd.eqm_species_charge[eqm_j]);
}

void
GeochemicalSystem::enforceChargeBalanceIfSimple(std::vector<Real> & constraint_value,
                                                std::vector<Real> & bulk_moles_old) const
{
  Real tot_charge = 0.0;
  for (unsigned basis_i = 0; basis_i < _num_basis; ++basis_i)
    if (_mgd.basis_species_charge[basis_i] != 0.0)
    {
      if (_constraint_meaning[basis_i] == ConstraintMeaningEnum::MOLES_BULK_SPECIES)
        tot_charge += _mgd.basis_species_charge[basis_i] * constraint_value[basis_i];
      else
        return;
    }
  // kinetic species are counted in the bulk
  // all charged basis species must have been provided with a MOLES_BULK_SPECIES value, so we can
  // easily enforce charge neutrality
  tot_charge -= _mgd.basis_species_charge[_charge_balance_basis_index] *
                constraint_value[_charge_balance_basis_index];
  constraint_value[_charge_balance_basis_index] =
      -tot_charge / _mgd.basis_species_charge[_charge_balance_basis_index];
  bulk_moles_old[_charge_balance_basis_index] = constraint_value[_charge_balance_basis_index];
}

Real
GeochemicalSystem::getTotalChargeOld() const
{
  Real tot_charge = 0.0;
  for (unsigned basis_i = 0; basis_i < _num_basis; ++basis_i)
    tot_charge += _mgd.basis_species_charge[basis_i] * _bulk_moles_old[basis_i];
  // kinetic species already counted in bulk_moles
  return tot_charge;
}

void
GeochemicalSystem::enforceChargeBalance()
{
  enforceChargeBalance(_constraint_value, _bulk_moles_old);
}

void
GeochemicalSystem::enforceChargeBalance(std::vector<Real> & constraint_value,
                                        std::vector<Real> & bulk_moles_old) const
{
  const Real tot_charge = getTotalChargeOld();
  constraint_value[_charge_balance_basis_index] -=
      tot_charge / _mgd.basis_species_charge[_charge_balance_basis_index];
  bulk_moles_old[_charge_balance_basis_index] = constraint_value[_charge_balance_basis_index];
}

void
GeochemicalSystem::setAlgebraicVariables(const DenseVector<Real> & algebraic_var)
{
  if (algebraic_var.size() != _num_in_algebraic_system)
    mooseError("Incorrect size in setAlgebraicVariables");
  for (unsigned a = 0; a < _num_in_algebraic_system; ++a)
    if (algebraic_var(a) <= 0.0)
      mooseError("Cannot set algebraic variables to non-positive values such as ",
                 algebraic_var(a));

  for (unsigned a = 0; a < _num_basis_in_algebraic_system; ++a)
    _basis_molality[_basis_index[a]] = algebraic_var(a);
  for (unsigned s = 0; s < _num_surface_pot; ++s)
    _surface_pot_expr[s] = algebraic_var(s + _num_basis_in_algebraic_system);
  for (unsigned k = 0; k < _num_kin; ++k)
    _kin_moles[k] = algebraic_var(k + _num_basis_in_algebraic_system + _num_surface_pot);

  computeConsistentConfiguration();
}

void
GeochemicalSystem::computeBulk(std::vector<Real> & bulk_moles_old) const
{
  for (unsigned i = 0; i < _num_basis; ++i)
  {
    const Real value = _constraint_value[i];
    const ConstraintMeaningEnum meaning = _constraint_meaning[i];
    switch (meaning)
    {
      case ConstraintMeaningEnum::MOLES_BULK_SPECIES:
      case ConstraintMeaningEnum::MOLES_BULK_WATER:
      {
        bulk_moles_old[i] = value;
        break;
      }
      case ConstraintMeaningEnum::KG_SOLVENT_WATER:
      case ConstraintMeaningEnum::FREE_MOLALITY:
      case ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES:
      case ConstraintMeaningEnum::FUGACITY:
      case ConstraintMeaningEnum::ACTIVITY:
      {
        bulk_moles_old[i] = computeBulkFromMolalities(i);
        break;
      }
    }
  }
}

Real
GeochemicalSystem::computeBulkFromMolalities(unsigned basis_ind) const
{
  const Real nw = _basis_molality[0];
  Real bulk = 0.0;
  if (basis_ind == 0)
    bulk = GeochemistryConstants::MOLES_PER_KG_WATER;
  else if (_mgd.basis_species_mineral[basis_ind])
    bulk = _basis_molality[basis_ind] / nw; // because of multiplication by nw, below
  else
    bulk = _basis_molality[basis_ind];
  for (unsigned j = 0; j < _num_eqm; ++j)
    bulk += _mgd.eqm_stoichiometry(j, basis_ind) * _eqm_molality[j];
  bulk *= nw;
  for (unsigned kin = 0; kin < _num_kin; ++kin)
    bulk += _mgd.kin_stoichiometry(kin, basis_ind) * _kin_moles[kin];
  return bulk;
}

void
GeochemicalSystem::computeTransportedBulkFromMolalities(std::vector<Real> & transported_bulk) const
{
  transported_bulk.resize(_num_basis);
  const Real nw = _basis_molality[0];
  for (unsigned i = 0; i < _num_basis; ++i)
  {
    transported_bulk[i] = 0.0;
    if (i == 0)
      transported_bulk[i] = GeochemistryConstants::MOLES_PER_KG_WATER;
    else if (_mgd.basis_species_transported[i])
    {
      if (_mgd.basis_species_mineral[i])
        transported_bulk[i] = _basis_molality[i] / nw; // because of multiplication by nw, below
      else
        transported_bulk[i] = _basis_molality[i];
    }
    else
      transported_bulk[i] = 0.0;
    for (unsigned j = 0; j < _num_eqm; ++j)
      if (_mgd.eqm_species_transported[j])
        transported_bulk[i] += _mgd.eqm_stoichiometry(j, i) * _eqm_molality[j];
    transported_bulk[i] *= nw;
  }
}

Real
GeochemicalSystem::getResidualComponent(unsigned algebraic_ind,
                                        const DenseVector<Real> & mole_additions) const
{
  if (algebraic_ind >= _num_in_algebraic_system)
    mooseError("Cannot retrieve residual for algebraic index ",
               algebraic_ind,
               " because there are only ",
               _num_basis_in_algebraic_system,
               " molalities in the algebraic system and ",
               _num_surface_pot,
               " surface potentials and ",
               _num_kin,
               " kinetic species");
  if (mole_additions.size() != _num_basis + _num_kin)
    mooseError("The increment in mole numbers (mole_additions) needs to be of size ",
               _num_basis,
               " + ",
               _num_kin,
               " but it is of size ",
               mole_additions.size());

  if (algebraic_ind < _num_basis_in_algebraic_system) // residual for basis molality
  {
    /*
     * Without the special things for water or the charge-balance species, we're trying to solve
     * bulk_new = bulk_old + mole_addition
     * where bulk_old is known (from previous time-step or, for a steady-state problem, the
     * constraint)
     * and mole_addition is given to this function
     * and bulk_new = nw * (m + sum_eqm[stoi * mol_eqm]) + sum_kin[stoi * mole_kin]
     * Hence, the residual is
     * R = -(bulk_old + mole_addition) + nw*(m + sum_eqm[stoi*mol_eqm]) + sum_kin[stoi*mole_kin]
     * This is seemingly different from Bethke Eqns(16.7)-(16.9), because Bethke bulk ("M") do not
     contain the sum_kin[stoi * mol_kin] terms.  Converting the above residual to Bethke's
     convention:
     * R = -((nw*(m + sum_eqm[stoi*mol_eqm]) + sum_kin[stoi*mole_kin])_old + mole_addition) + nw*(m
     + sum_eqm[stoi*mol_eqm]) + sum_kin[stoi*mole_kin]
     *   = -(M_old + sum_kin[stoi*mole_kin_old] + mole_addition) + M + sum_kin[stoi*mole_kin]
     *   = -(M_old + mole_addition) + M + sum_kin[stoi*kin_mole_addition]
     * which is exactly Eqns(16.7)-(16.9).
     * One problem with Bethke's approach is that if kin_mole_addition depends on the mole number
     of the kinetic species, there is a "hidden" variable (moles of kinetic species) which is kept
     constant during the solve.  Here, including the moles of kinetic species as additional
     variables allows greater accuracy.
     */
    const unsigned basis_i = _basis_index[algebraic_ind];
    Real res = 0.0;
    if (basis_i == 0)
      res += -(_bulk_moles_old[basis_i] + mole_additions(basis_i)) +
             _basis_molality[0] * GeochemistryConstants::MOLES_PER_KG_WATER;
    else if (basis_i == _charge_balance_basis_index)
    {
      res += _basis_molality[0] * _basis_molality[basis_i];
      for (unsigned i = 0; i < _num_basis; ++i)
      {
        if (i == _charge_balance_basis_index)
          continue;
        else if (_mgd.basis_species_charge[i] ==
                 0.0) // certainly includes water, minerals and gases
          continue;
        else if (_constraint_meaning[i] == ConstraintMeaningEnum::MOLES_BULK_SPECIES)
        {
          // the molalities might not yet have converged so that the bulk moles of this species
          // currently equals _bulk_moles_old + mole_additions, but we know
          // that when the solve has converged it'll have to, so:
          res += _mgd.basis_species_charge[i] * (_bulk_moles_old[i] + mole_additions(i)) /
                 _mgd.basis_species_charge[basis_i];
        }
        else
        {
          // do not know the bulk moles of this species: use the current value for molality and
          // kin_moles.  Note, there is no mole_additions here since physically any mole_additions
          // get instantly get soaked up by the fixed activity (or fixed molality, etc), and
          // mathematically when we eventually come to add mole_additions to the bulk_moles_old
          // (via addtoBulkMoles, for instance) we immediately return without adding stuff.  End
          // result: charge-neutrality should not depend on mole_additions for fixed-activity
          // (molality, etc) species.
          res += _mgd.basis_species_charge[i] * computeBulkFromMolalities(i) /
                 _mgd.basis_species_charge[basis_i];
        }
      }
    }
    else
      res += -(_bulk_moles_old[basis_i] + mole_additions(basis_i)) +
             _basis_molality[0] * _basis_molality[basis_i];
    for (unsigned eqm_j = 0; eqm_j < _num_eqm; ++eqm_j)
      res += _basis_molality[0] * _mgd.eqm_stoichiometry(eqm_j, basis_i) * _eqm_molality[eqm_j];
    for (unsigned kin = 0; kin < _num_kin; ++kin)
      res += _mgd.kin_stoichiometry(kin, basis_i) * _kin_moles[kin];
    return res;
  }
  else if (algebraic_ind <
           _num_basis_in_algebraic_system + _num_surface_pot) // residual for surface potential
  {
    const unsigned sp = algebraic_ind - _num_basis_in_algebraic_system;
    Real res = surfacePotPrefactor(sp) * (_surface_pot_expr[sp] - 1.0 / _surface_pot_expr[sp]);
    for (unsigned j = 0; j < _num_eqm; ++j)
      if (_mgd.surface_sorption_related[j] && _mgd.surface_sorption_number[j] == sp)
        res += _basis_molality[0] * _mgd.eqm_species_charge[j] * _eqm_molality[j];
    return res;
  }

  // else: residual for kinetic
  const unsigned kin = algebraic_ind - _num_basis_in_algebraic_system - _num_surface_pot;
  Real res = _kin_moles[kin] - (_kin_moles_old[kin] + mole_additions(_num_basis + kin));
  return res;
}

void
GeochemicalSystem::computeJacobian(const DenseVector<Real> & res,
                                   DenseMatrix<Real> & jac,
                                   const DenseVector<Real> & mole_additions,
                                   const DenseMatrix<Real> & dmole_additions) const
{
  /* To the reader: yes, this is awfully complicated.  The best way of understanding it is to very
   * slowly go through the residual calculation and compute the derivatives by hand.  It is quite
   * well tested, but it'd be great if you could add more tests!
   */
  if (res.size() != _num_in_algebraic_system)
    mooseError(
        "Jacobian: residual size must be ", _num_in_algebraic_system, " but it is ", res.size());
  if (mole_additions.size() != _num_basis + _num_kin)
    mooseError("Jacobian: the increment in mole numbers (mole_additions) needs to be of size ",
               _num_basis,
               " + ",
               _num_kin,
               " but it is of size ",
               mole_additions.size());
  if (!(dmole_additions.n() == _num_basis + _num_kin &&
        dmole_additions.m() == _num_basis + _num_kin))
    mooseError("Jacobian: the derivative of mole additions (dmole_additions) needs to be of size ",
               _num_basis + _num_kin,
               "x",
               _num_basis + _num_kin,
               " but it is of size ",
               dmole_additions.m(),
               "x",
               dmole_additions.n());
  /*
Note that in this function we make use of the fact that species can only be in the algebraic
system if their molality is unknown (or in the case of water, the mass of solvent mass is
unknown).  This means that the molality of the equilibrium species depends on
(activity_coefficient * basis molality)^stoi, so that the derivative is quite simple.  Eg, we
don't have to worry about derivatives with respect to fixed-activity things, or fixed fugacity.
Also, note that the constructor and the various "set" methods of this class enforce molality > 0,
so there are no division-by-zero problems.  Also, note that we never compute derivatives of the
activity coefficients, or the activity of water with respect to the molalities, as they are
assumed to be quite small.  Finally, the surface_pot_expr will also always be positive.
  */
  // correctly size and zero
  jac.resize(_num_in_algebraic_system, _num_in_algebraic_system);
  const Real nw = _basis_molality[0];

  // jac(a, b) = d(R_a) / d(m_b), where a corresponds to a molality
  for (unsigned a = 0; a < _num_basis_in_algebraic_system; ++a)
  {
    const unsigned basis_of_a = _basis_index[a];
    for (unsigned b = 0; b < _num_basis_in_algebraic_system; ++b)
    {
      const unsigned basis_of_b = _basis_index[b];

      // contribution from mole_additions
      if (basis_of_a != _charge_balance_basis_index)
        jac(a, b) -= dmole_additions(basis_of_a, basis_of_b);
      else
      {
        for (unsigned i = 0; i < _num_basis; ++i)
        {
          if (i == _charge_balance_basis_index)
            continue;
          else if (_mgd.basis_species_charge[i] ==
                   0.0) // certainly includes water, minerals and gases
            continue;
          else if (_constraint_meaning[i] == ConstraintMeaningEnum::MOLES_BULK_SPECIES)
            jac(a, b) += _mgd.basis_species_charge[i] * dmole_additions(i, basis_of_b) /
                         _mgd.basis_species_charge[basis_of_a];
        }
      }

      // contribution from explicit nw, in case where mass of solvent water is an unknown
      if (basis_of_b == 0)
      {
        if (basis_of_a != _charge_balance_basis_index)
        {
          // use a short-cut here: dR/dnw = m + sum_eqm[stoi*mol] = (R + bulk + addition - kin)/nw
          Real numerator = res(a) + _bulk_moles_old[basis_of_a] + mole_additions(basis_of_a);
          for (unsigned kin = 0; kin < _num_kin; ++kin)
            numerator -= _mgd.kin_stoichiometry(kin, basis_of_a) * _kin_moles[kin];
          jac(a, 0) += numerator / nw;
        }
        else
        {
          for (unsigned i = 0; i < _num_basis; ++i)
          {
            if (i == _charge_balance_basis_index)
              continue;
            else if (_mgd.basis_species_charge[i] ==
                     0.0) // certainly includes water, minerals and gases
              continue;
            else if (_constraint_meaning[i] == ConstraintMeaningEnum::MOLES_BULK_SPECIES)
              continue;
            else
            {
              Real molal = _basis_molality[i];
              for (unsigned j = 0; j < _num_eqm; ++j)
                molal += _mgd.eqm_stoichiometry(j, i) * _eqm_molality[j];
              jac(a, 0) +=
                  _mgd.basis_species_charge[i] * molal / _mgd.basis_species_charge[basis_of_a];
            }
          }
          jac(a, 0) += _basis_molality[basis_of_a];
          for (unsigned j = 0; j < _num_eqm; ++j)
            jac(a, 0) += _mgd.eqm_stoichiometry(j, basis_of_a) * _eqm_molality[j];
        }
      }

      // contribution from molality of basis_of_b
      if (basis_of_b != 0)
      {
        if (a == b)
          jac(a, b) += nw; // remember b != 0
        for (unsigned eqm_j = 0; eqm_j < _num_eqm; ++eqm_j)
        {
          jac(a, b) += nw * _mgd.eqm_stoichiometry(eqm_j, basis_of_a) * _eqm_molality[eqm_j] *
                       _mgd.eqm_stoichiometry(eqm_j, basis_of_b) / _basis_molality[basis_of_b];
        }
        if (basis_of_a == _charge_balance_basis_index)
        {
          // additional terms from the charge-balance additions
          for (unsigned i = 0; i < _num_basis; ++i)
          {
            if (i == _charge_balance_basis_index)
              continue;
            else if (_mgd.basis_species_charge[i] ==
                     0.0) // certainly includes water, minerals and gases
              continue;
            else if (_constraint_meaning[i] == ConstraintMeaningEnum::MOLES_BULK_SPECIES)
              continue;
            else
            {
              const Real prefactor =
                  _mgd.basis_species_charge[i] * nw / _mgd.basis_species_charge[basis_of_a];
              if (i == basis_of_b)
                jac(a, b) += prefactor;
              for (unsigned eqm_j = 0; eqm_j < _num_eqm; ++eqm_j)
              {
                jac(a, b) += prefactor * _mgd.eqm_stoichiometry(eqm_j, i) * _eqm_molality[eqm_j] *
                             _mgd.eqm_stoichiometry(eqm_j, basis_of_b) /
                             _basis_molality[basis_of_b];
              }
            }
          }
        }
      }
    }
  }

  // jac(a, b) = d(R_a) / d(surface_pot), where a corresponds to a molality
  for (unsigned a = 0; a < _num_basis_in_algebraic_system; ++a)
  {
    const unsigned basis_of_a = _basis_index[a];
    for (unsigned s = 0; s < _num_surface_pot; ++s)
    {
      const unsigned b = s + _num_basis_in_algebraic_system;
      // derivative of nw * _mgd.eqm_stoichiometry(eqm_j, basis_i) * _eqm_molality[eqm_j],
      // where _eqm_molality = (_surface_pot_expr)^(2 * charge) * stuff
      for (unsigned eqm_j = 0; eqm_j < _num_eqm; ++eqm_j)
        if (_mgd.surface_sorption_related[eqm_j] && _mgd.surface_sorption_number[eqm_j] == s)
          jac(a, b) += nw * _mgd.eqm_stoichiometry(eqm_j, basis_of_a) * 2.0 *
                       _mgd.eqm_species_charge[eqm_j] * _eqm_molality[eqm_j] /
                       _surface_pot_expr[_mgd.surface_sorption_number[eqm_j]];
    }
  }

  // jac(a, b) = d(R_a) / d(_kin_moles) where a corresponds to a molality
  for (unsigned a = 0; a < _num_basis_in_algebraic_system; ++a)
  {
    const unsigned basis_of_a = _basis_index[a];

    // contribution from mole_additions
    for (unsigned kin = 0; kin < _num_kin; ++kin)
    {
      const unsigned ind = _num_basis + kin;
      const unsigned b = _num_basis_in_algebraic_system + _num_surface_pot + kin;
      if (basis_of_a != _charge_balance_basis_index)
        jac(a, b) -= dmole_additions(basis_of_a, ind);
      else
      {
        for (unsigned i = 0; i < _num_basis; ++i)
        {
          if (i == _charge_balance_basis_index)
            continue;
          else if (_mgd.basis_species_charge[i] ==
                   0.0) // certainly includes water, minerals and gases
            continue;
          else if (_constraint_meaning[i] == ConstraintMeaningEnum::MOLES_BULK_SPECIES)
            jac(a, b) += _mgd.basis_species_charge[i] * dmole_additions(i, ind) /
                         _mgd.basis_species_charge[basis_of_a];
        }
      }
    }

    // contribution from sum_kin[stoi*kin_moles]
    for (unsigned kin = 0; kin < _num_kin; ++kin)
    {
      const unsigned b = _num_basis_in_algebraic_system + _num_surface_pot + kin;
      jac(a, b) += _mgd.kin_stoichiometry(kin, basis_of_a);
    }

    // special additional contribution for charge-balance from kinetic stuff
    if (basis_of_a == _charge_balance_basis_index)
    {
      for (unsigned i = 0; i < _num_basis; ++i)
      {
        if (i == _charge_balance_basis_index)
          continue;
        else if (_mgd.basis_species_charge[i] ==
                 0.0) // certainly includes water, minerals and gases
          continue;
        else if (_constraint_meaning[i] == ConstraintMeaningEnum::MOLES_BULK_SPECIES)
          continue;
        else
        {
          const Real prefactor =
              _mgd.basis_species_charge[i] / _mgd.basis_species_charge[basis_of_a];
          for (unsigned kin = 0; kin < _num_kin; ++kin)
          {
            const unsigned b = _num_basis_in_algebraic_system + _num_surface_pot + kin;
            jac(a, b) += prefactor * _mgd.kin_stoichiometry(kin, i);
          }
        }
      }
    }
  }

  // jac(a, b) = d(R_a) / d(variable_b) where a corresponds to a surface potential
  for (unsigned s = 0; s < _num_surface_pot; ++s)
  {
    const unsigned a = s + _num_basis_in_algebraic_system;

    for (unsigned b = 0; b < _num_basis_in_algebraic_system; ++b)
    {
      // derivative of nw * _mgd.eqm_species_charge[j] * _eqm_molality[j];
      const unsigned basis_of_b = _basis_index[b];
      if (basis_of_b == 0) // special case: mass of solvent water is an unknown
      {
        for (unsigned j = 0; j < _num_eqm; ++j)
          if (_mgd.surface_sorption_related[j] && _mgd.surface_sorption_number[j] == s)
            jac(a, b) += _mgd.eqm_species_charge[j] * _eqm_molality[j];
      }
      else
      {
        for (unsigned j = 0; j < _num_eqm; ++j)
          if (_mgd.surface_sorption_related[j] && _mgd.surface_sorption_number[j] == s)
            jac(a, b) += nw * _mgd.eqm_species_charge[j] * _eqm_molality[j] *
                         _mgd.eqm_stoichiometry(j, basis_of_b) / _basis_molality[basis_of_b];
      }
    }

    const Real coef = surfacePotPrefactor(s);
    // derivative of coef * (x - 1/x) wrt x, where x = _surface_pot_expr
    jac(a, a) += coef * (1.0 + 1.0 / std::pow(_surface_pot_expr[s], 2.0));
    // derivative of nw * _mgd.eqm_species_charge[j] * _eqm_molality[j];
    // where _eqm_molality = (_surface_pot_expr)^(2 * charge) * stuff
    for (unsigned j = 0; j < _num_eqm; ++j)
      if (_mgd.surface_sorption_related[j] && _mgd.surface_sorption_number[j] == s)
        jac(a, a) += nw * std::pow(_mgd.eqm_species_charge[j], 2.0) * 2.0 * _eqm_molality[j] /
                     _surface_pot_expr[s];
  }

  // jac(a, b) = d(R_a) / d(variable_b) where a corresponds to a kinetic species
  for (unsigned kin = 0; kin < _num_kin; ++kin)
  {
    const unsigned a = _num_basis_in_algebraic_system + _num_surface_pot + kin;
    jac(a, a) += 1.0; // deriv wrt _kin_moles[kin]

    // contribution from mole_additions
    const unsigned ind_of_addition = _num_basis + kin;
    for (unsigned b = 0; b < _num_basis_in_algebraic_system; ++b)
    {
      const unsigned basis_of_b = _basis_index[b];
      jac(a, b) -= dmole_additions(ind_of_addition, basis_of_b);
    }
    for (unsigned kinp = 0; kinp < _num_kin; ++kinp)
    {
      const unsigned b = _num_basis_in_algebraic_system + _num_surface_pot + kinp;
      const unsigned indp = _num_basis + kinp;
      jac(a, b) -= dmole_additions(ind_of_addition, indp);
    }
  }
}

const ModelGeochemicalDatabase &
GeochemicalSystem::getModelGeochemicalDatabase() const
{
  return _mgd;
}

void
GeochemicalSystem::computeFreeMineralMoles(std::vector<Real> & basis_molality) const
{

  const Real nw = _basis_molality[0];
  for (unsigned i = 0; i < _num_basis; ++i)
    if (_mgd.basis_species_mineral[i])
    {
      if (_constraint_meaning[i] == ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES)
        basis_molality[i] = _constraint_value[i];
      else
      {
        basis_molality[i] = _bulk_moles_old[i];
        for (unsigned j = 0; j < _num_eqm; ++j)
          basis_molality[i] -= nw * _mgd.eqm_stoichiometry(j, i) * _eqm_molality[j];
        for (unsigned kin = 0; kin < _num_kin; ++kin)
          basis_molality[i] -= _mgd.kin_stoichiometry(kin, i) * _kin_moles[kin];
      }
    }
}

std::vector<Real>
GeochemicalSystem::getSaturationIndices() const
{
  std::vector<Real> si(_num_eqm);
  for (unsigned j = 0; j < _num_eqm; ++j)
    if (_mgd.eqm_species_mineral[j])
      si[j] = log10ActivityProduct(j) - _eqm_log10K[j];
    else
      si[j] = 0.0;
  return si;
}

void
GeochemicalSystem::performSwap(unsigned swap_out_of_basis, unsigned swap_into_basis)
{
  if (swap_out_of_basis == 0)
    mooseException("GeochemicalSystem: attempting to swap out water and replace it by ",
                   _mgd.eqm_species_name[swap_into_basis],
                   ".  This could be because the algorithm would like to "
                   "swap out the charge-balance species, in which case you should choose a "
                   "different charge-balance species");
  if (swap_out_of_basis == _charge_balance_basis_index)
    mooseException("GeochemicalSystem: attempting to swap the charge-balance species out of "
                   "the basis");
  if (_mgd.basis_species_gas[swap_out_of_basis])
    mooseException("GeochemicalSystem: attempting to swap a gas out of the basis");
  if (_mgd.eqm_species_gas[swap_into_basis])
    mooseException("GeochemicalSystem: attempting to swap a gas into the basis");
  // perform the swap
  performSwapNoCheck(swap_out_of_basis, swap_into_basis);
}

void
GeochemicalSystem::performSwapNoCheck(unsigned swap_out_of_basis, unsigned swap_into_basis)
{
  DenseVector<Real> bm = _bulk_moles_old;
  _swapper.performSwap(_mgd, bm, swap_out_of_basis, swap_into_basis);

  // the swap_into_basis species now has fixed bulk moles irrespective of what the
  // swap_out_of_basis species had fixed
  _constraint_meaning[swap_out_of_basis] = ConstraintMeaningEnum::MOLES_BULK_SPECIES;

  // the bulk moles will have changed for many components.  The _bulk_moles_old values will be
  // set in computeConsistentConfiguration, below
  for (unsigned basis_i = 0; basis_i < _num_basis; ++basis_i)
    if (_constraint_meaning[basis_i] == ConstraintMeaningEnum::MOLES_BULK_SPECIES ||
        _constraint_meaning[basis_i] == ConstraintMeaningEnum::MOLES_BULK_WATER)
    {
      _constraint_value[basis_i] = bm(basis_i);
      _original_constraint_value[basis_i] = bm(basis_i);
    }

  // In the following, the molalities are carefully swapped so that the configuration stays
  // reasonably close to the configuration prior to the swap.  This may be advantageous as it
  // initializes the Newton process with a better starting guess than what might have been used
  Real molality_of_species_just_swapped_in =
      _eqm_molality[swap_into_basis]; // is positive, or zero iff (mineral or gas)
  if (_mgd.basis_species_mineral[swap_out_of_basis] || _mgd.basis_species_gas[swap_out_of_basis] ||
      _eqm_molality[swap_into_basis] == 0.0)
  {
    // the species just swapped in is a mineral or a gas, so its equilibium molality is
    // undefined: make a guess for its molality
    molality_of_species_just_swapped_in =
        std::max(_min_initial_molality, 0.9 * bm(swap_out_of_basis));
  }
  Real molality_of_species_just_swapped_out =
      _basis_molality[swap_out_of_basis]; // can be negative if a consumed mineral
  _basis_molality[swap_out_of_basis] = molality_of_species_just_swapped_in;
  _eqm_molality[swap_into_basis] =
      std::max(0.0, molality_of_species_just_swapped_out); // this gets recalculated in
                                                           // computeConsistentConfiguration, below

  // depending if minerals were swapped in or out of the basis, the known activity may have
  // changed
  buildKnownBasisActivities(_basis_activity_known, _basis_activity);

  // the equilibrium constants will have changed due to the swap
  buildTemperatureDependentQuantities(_temperature);

  // due to re-orderings in mgd, the charge-balance basis index might have changed
  _charge_balance_basis_index = _mgd.basis_species_index.at(_charge_balance_species);
  // charge balance might be able to be performed easily
  enforceChargeBalanceIfSimple(_constraint_value, _bulk_moles_old);

  // the algebraic system has probably changed
  buildAlgebraicInfo(_in_algebraic_system,
                     _num_basis_in_algebraic_system,
                     _num_in_algebraic_system,
                     _algebraic_index,
                     _basis_index);

  // finally compute a consistent configuration, based on the basis molalities, etc, above
  computeConsistentConfiguration();
}

unsigned
GeochemicalSystem::getNumInBasis() const
{
  return _num_basis;
}

unsigned
GeochemicalSystem::getNumInEquilibrium() const
{
  return _num_eqm;
}

void
GeochemicalSystem::setChargeBalanceSpecies(unsigned new_charge_balance_index)
{
  // because the original mole number of the charge balance species may have been changed due to
  // enforcing charge balance:
  _constraint_value[_charge_balance_basis_index] =
      _original_constraint_value[_charge_balance_basis_index];
  _bulk_moles_old[_charge_balance_basis_index] = _constraint_value[_charge_balance_basis_index];
  // now change the charge balance info
  _charge_balance_basis_index = new_charge_balance_index;
  _charge_balance_species = _mgd.basis_species_name[_charge_balance_basis_index];
  // enforce charge-balance if easily possible
  enforceChargeBalanceIfSimple(_constraint_value, _bulk_moles_old);
}

bool
GeochemicalSystem::alterChargeBalanceSpecies(Real threshold_molality)
{
  if (_basis_molality[_charge_balance_basis_index] > threshold_molality)
    return false;
  unsigned best_species_opp_sign = 0;
  unsigned best_species_same_sign = 0;
  Real best_molality_opp_sign = 0.0;
  Real best_molality_same_sign = 0.0;
  for (unsigned basis_i = 0; basis_i < _num_basis; ++basis_i)
  {
    if (basis_i == _charge_balance_basis_index)
      continue;
    else if (_constraint_meaning[basis_i] != ConstraintMeaningEnum::MOLES_BULK_SPECIES)
      continue;
    else if (_mgd.basis_species_charge[basis_i] == 0.0)
      continue;
    else if (_basis_molality[basis_i] <= threshold_molality)
      continue;
    // we know basis_i is a charged species with set bulk moles and high molality
    if (_mgd.basis_species_charge[basis_i] *
            _mgd.basis_species_charge[_charge_balance_basis_index] <
        0.0)
    {
      // charge of opposite sign
      if (_basis_molality[basis_i] > best_molality_opp_sign)
      {
        best_species_opp_sign = basis_i;
        best_molality_opp_sign = _basis_molality[basis_i];
      }
    }
    else
    {
      // charge of same sign
      if (_basis_molality[basis_i] > best_molality_same_sign)
      {
        best_species_same_sign = basis_i;
        best_molality_same_sign = _basis_molality[basis_i];
      }
    }
  }
  if (best_species_opp_sign != 0)
  {
    // this is preferred over the same-sign version
    setChargeBalanceSpecies(best_species_opp_sign);
    return true;
  }
  else if (best_species_same_sign != 0)
  {
    // this is not preferred, but is better than no charge-balance species change
    setChargeBalanceSpecies(best_species_same_sign);
    return true;
  }
  return false;
}

bool
GeochemicalSystem::revertToOriginalChargeBalanceSpecies()
{
  if (_mgd.basis_species_index.find(_original_charge_balance_species) ==
      _mgd.basis_species_index.end())
    return false; // original charge-balance species no longer in basis
  const unsigned original_index = _mgd.basis_species_index.at(_original_charge_balance_species);
  if (original_index == _charge_balance_basis_index)
    return false; // current charge-balance species is the original
  setChargeBalanceSpecies(original_index);
  return true;
}

Real
GeochemicalSystem::getIonicStrength() const
{
  return _is.ionicStrength(_mgd, _basis_molality, _eqm_molality, _kin_moles);
}

Real
GeochemicalSystem::getStoichiometricIonicStrength() const
{
  return _is.stoichiometricIonicStrength(_mgd, _basis_molality, _eqm_molality, _kin_moles);
}

Real
GeochemicalSystem::surfacePotPrefactor(unsigned sp) const
{
  return 0.5 * _sorbing_surface_area[sp] / GeochemistryConstants::FARADAY *
         std::sqrt(8.0 * GeochemistryConstants::GAS_CONSTANT *
                   (_temperature + GeochemistryConstants::CELSIUS_TO_KELVIN) *
                   GeochemistryConstants::PERMITTIVITY_FREE_SPACE *
                   GeochemistryConstants::DIELECTRIC_CONSTANT_WATER *
                   GeochemistryConstants::DENSITY_WATER *
                   _is.ionicStrength(_mgd, _basis_molality, _eqm_molality, _kin_moles));
}

Real
GeochemicalSystem::getSurfacePotential(unsigned sp) const
{
  if (sp >= _num_surface_pot)
    mooseError("Cannot retrieve the surface potential for surface ",
               sp,
               " since there are only ",
               _num_surface_pot,
               " surfaces involved in surface complexation");
  return -2.0 * GeochemistryConstants::GAS_CONSTANT *
         (_temperature + GeochemistryConstants::CELSIUS_TO_KELVIN) /
         GeochemistryConstants::FARADAY * std::log(_surface_pot_expr[sp]);
}

Real
GeochemicalSystem::getSurfaceCharge(unsigned sp) const
{
  if (sp >= _num_surface_pot)
    mooseError("Cannot retrieve the surface charge for surface ",
               sp,
               " since there are only ",
               _num_surface_pot,
               " surfaces involved in surface complexation");
  // pre = mol of charge per square metre.  To convert to Coulombs/m^2:
  const Real pre = surfacePotPrefactor(sp) / _sorbing_surface_area[sp] *
                   (-_surface_pot_expr[sp] + 1.0 / _surface_pot_expr[sp]);
  return pre * GeochemistryConstants::FARADAY;
}

void
GeochemicalSystem::computeSorbingSurfaceArea(std::vector<Real> & sorbing_surface_area) const
{
  for (unsigned sp = 0; sp < _num_surface_pot; ++sp)
  {
    sorbing_surface_area[sp] = _mgd.surface_sorption_area[sp];
    if (_mgd.basis_species_index.count(_mgd.surface_sorption_name[sp]) == 1)
    {
      const unsigned basis_ind = _mgd.basis_species_index.at(_mgd.surface_sorption_name[sp]);
      const Real grams =
          _mgd.basis_species_molecular_weight[basis_ind] * _basis_molality[basis_ind];
      sorbing_surface_area[sp] *= grams;
    }
  }
}

const std::vector<Real> &
GeochemicalSystem::getSorbingSurfaceArea() const
{
  return _sorbing_surface_area;
}

Real
GeochemicalSystem::getTemperature() const
{
  return _temperature;
}

void
GeochemicalSystem::setTemperature(Real temperature)
{
  _temperature = temperature;
  buildTemperatureDependentQuantities(_temperature);
  _gac.setInternalParameters(_temperature, _mgd, _basis_molality, _eqm_molality, _kin_moles);
}

void
GeochemicalSystem::setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles(
    const std::vector<std::string> & names,
    const std::vector<Real> & values,
    const std::vector<bool> & constraints_from_molalities)
{
  // assume temperature-dependent quantities have been built during instantiation
  // assume algebraic info has been built during instantiation

  /*
   * STEP 0: Check sizes are correct
   */
  const unsigned num_names = names.size();
  if (num_names != values.size())
    mooseError("When setting all molalities, names and values must have same size");
  if (num_names != _num_basis + _num_eqm + _num_surface_pot + _num_kin)
    mooseError("When setting all molalities, values must be provided for every species and surface "
               "potentials");
  if (constraints_from_molalities.size() != _num_basis)
    mooseError("constraints_from_molalities has size ",
               constraints_from_molalities.size(),
               " while number of basis species is ",
               _num_basis);

  /*
   * STEP 1: Read values into _basis_molality, _eqm_molality and _surface_pot_expr and
   * _kin_moles
   */
  // The is no guarantee is made that the user-supplied molalities are "good", except they must
  // be non-negative. There are lots of "finds" in the loops below, which is slow, but it
  // ensures all species are given molalities.  This method is designed to be called only every
  // once-in-a-while (eg, at the start of a simulation)
  for (const auto & name_index : _mgd.basis_species_index)
  {
    const unsigned ind =
        std::distance(names.begin(), std::find(names.begin(), names.end(), name_index.first));
    if (ind < num_names)
    {
      if (_mgd.basis_species_gas[name_index.second])
      {
        if (values[ind] != 0.0)
          mooseError("Molality for gas ",
                     name_index.first,
                     " cannot be ",
                     values[ind],
                     ": it must be zero");
        else
          _basis_molality[name_index.second] = values[ind];
      }
      else if (_mgd.basis_species_mineral[name_index.second])
      {
        if (values[ind] < 0.0)
          mooseError("Molality for mineral ",
                     name_index.first,
                     " cannot be ",
                     values[ind],
                     ": it must be non-negative");
        else
          _basis_molality[name_index.second] = values[ind];
      }
      else if (values[ind] <= 0.0)
        mooseError("Molality for species ",
                   name_index.first,
                   " cannot be ",
                   values[ind],
                   ": it must be positive");
      else
        _basis_molality[name_index.second] = values[ind];
    }
    else
      mooseError("Molality (or free mineral moles, etc - whatever is appropriate) for species ",
                 name_index.first,
                 " needs to be provided when setting all molalities");
  }
  for (const auto & name_index : _mgd.eqm_species_index)
  {
    const unsigned ind =
        std::distance(names.begin(), std::find(names.begin(), names.end(), name_index.first));
    if (ind < num_names)
    {
      if (_mgd.eqm_species_gas[name_index.second] || _mgd.eqm_species_mineral[name_index.second])
        _eqm_molality[name_index.second] =
            0.0; // Note: explicitly setting to zero, irrespective of user input.  The reason
                 // for doing this is that during a restore, a basis species with positive
                 // molality could become a secondary species, which should have zero molality
      else if (values[ind] < 0.0)
        mooseError("Molality for species ",
                   name_index.first,
                   " cannot be ",
                   values[ind],
                   ": it must be non-negative");
      else
        _eqm_molality[name_index.second] = values[ind];
    }
    else
      mooseError("Molality for species ",
                 name_index.first,
                 " needs to be provided when setting all molalities");
  }
  for (unsigned sp = 0; sp < _num_surface_pot; ++sp)
  {
    const unsigned ind =
        std::distance(names.begin(),
                      std::find(names.begin(),
                                names.end(),
                                _mgd.surface_sorption_name[sp] + "_surface_potential_expr"));
    if (ind < num_names)
    {
      if (values[ind] <= 0.0)
        mooseError("Surface-potential expression for mineral ",
                   _mgd.surface_sorption_name[sp],
                   " cannot be ",
                   values[ind],
                   ": it must be positive");
      _surface_pot_expr[sp] = values[ind];
    }
    else
      mooseError("Surface potential for mineral ",
                 _mgd.surface_sorption_name[sp],
                 " needs to be provided when setting all molalities");
  }
  for (const auto & name_index : _mgd.kin_species_index)
  {
    const unsigned ind =
        std::distance(names.begin(), std::find(names.begin(), names.end(), name_index.first));
    if (ind < num_names)
      setKineticMoles(name_index.second, values[ind]);
    else
      mooseError("Moles for species ",
                 name_index.first,
                 " needs to be provided when setting all molalities");
  }

  /*
   * STEP 2: Alter _constraint_values if necessary
   */
  // If some of the constraints_from_molalities are false, then the molalities provided to this
  // method may have to be modified to satisfy the contraints: this alters _basis_molality so
  // must occur first
  for (unsigned i = 0; i < _num_basis; ++i)
  {
    const ConstraintMeaningEnum meaning = _constraint_meaning[i];
    if (meaning == ConstraintMeaningEnum::KG_SOLVENT_WATER ||
        meaning == ConstraintMeaningEnum::FREE_MOLALITY ||
        meaning == ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES)
    {
      if (constraints_from_molalities[i])
      {
        // molalities provided to this method dictate the contraints:
        _constraint_value[i] = _basis_molality[i];
        _original_constraint_value[i] = _constraint_value[i];
      }
      else
        // contraints take precedence over the molalities provided to this method:
        _basis_molality[i] = _constraint_value[i];
    }
  }

  // Potentially alter _constraint_value for the BULK contraints:
  for (unsigned i = 0; i < _num_basis; ++i)
  {
    const ConstraintMeaningEnum meaning = _constraint_meaning[i];
    if (meaning == ConstraintMeaningEnum::MOLES_BULK_WATER ||
        meaning == ConstraintMeaningEnum::MOLES_BULK_SPECIES)
      if (constraints_from_molalities[i])
      {
        // the constraint value should be overridden by the molality-computed bulk mole number
        _constraint_value[i] = computeBulkFromMolalities(i);
        _original_constraint_value[i] = _constraint_value[i];
      }
  }

  // Potentially alter _constraint_value for ACTIVITY contraints:
  for (unsigned i = 0; i < _num_basis; ++i)
    if (constraints_from_molalities[i])
    {
      const ConstraintMeaningEnum meaning = _constraint_meaning[i];
      if (meaning == ConstraintMeaningEnum::FUGACITY)
        mooseError("Gas fugacity cannot be determined from molality: constraints_from_molalities "
                   "must be set false for all gases");
      else if (meaning == ConstraintMeaningEnum::ACTIVITY)
      {
        if (i == 0)
          mooseError("Water activity cannot be determined from molalities: "
                     "constraints_from_molalities "
                     "must be set to false for water if activity of water is fixed");
        // the constraint value should be overidden by the molality provided to this method
        _constraint_value[i] = _basis_activity_coef[i] * _basis_molality[i];
        _original_constraint_value[i] = _constraint_value[i];
      }
    }

  /*
   * STEP 3: Follow the initialize() and computeConsistentConfiguration() methods
   */
  // assume done already: buildTemperatureDependentQuantities
  enforceChargeBalanceIfSimple(_constraint_value, _bulk_moles_old);
  // assume done already: buildAlgebraicInfo
  // should not be done, as basis_molality is set by this method instead: initBulkAndFree
  buildKnownBasisActivities(_basis_activity_known, _basis_activity);
  // should not be done, as these are set by this method: _eqm_molality.assign
  // should not be done, as these are set by this method: _surface_pot_expr.assign
  _gac.setInternalParameters(_temperature, _mgd, _basis_molality, _eqm_molality, _kin_moles);
  _gac.buildActivityCoefficients(_mgd, _basis_activity_coef, _eqm_activity_coef);
  // for constraints_from_molalities = false then the following: (1) overrides the basis
  // molality provided to this method; (2) produces a slightly inconsistent equilibrium
  // geochemical system because basis_activity_coef was computed on the basis of the basis
  // molalities provided to this method
  updateBasisMolalityForKnownActivity(_basis_molality);
  computeRemainingBasisActivities(_basis_activity);
  // should not be done, as these are set by this method: computeEqmMolalities
  computeBulk(_bulk_moles_old);
  // should not be done, as these are set by this method: computeFreeMineralMoles
  computeSorbingSurfaceArea(_sorbing_surface_area);
}

const std::vector<GeochemicalSystem::ConstraintMeaningEnum> &
GeochemicalSystem::getConstraintMeaning() const
{
  return _constraint_meaning;
}

void
GeochemicalSystem::closeSystem()
{
  for (unsigned basis_ind = 0; basis_ind < _num_basis; ++basis_ind)
    if (_constraint_meaning[basis_ind] == ConstraintMeaningEnum::KG_SOLVENT_WATER ||
        _constraint_meaning[basis_ind] == ConstraintMeaningEnum::FREE_MOLALITY ||
        _constraint_meaning[basis_ind] == ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES)
      changeConstraintToBulk(basis_ind);
}

void
GeochemicalSystem::changeConstraintToBulk(unsigned basis_ind)
{
  if (basis_ind >= _num_basis)
    mooseError("Cannot changeConstraintToBulk for species ",
               basis_ind,
               " because there are only ",
               _num_basis,
               " basis species");
  if (_mgd.basis_species_gas[basis_ind])
  {
    // this is a special case where we have to swap out the gas in favour of an equilibrium
    // species
    unsigned swap_into_basis = 0;
    bool legitimate_swap_found = false;
    Real best_stoi = 0.0;
    for (unsigned j = 0; j < _num_eqm; ++j)
    {
      if (_mgd.eqm_species_gas[j] || _mgd.eqm_stoichiometry(j, basis_ind) == 0.0 ||
          _mgd.surface_sorption_related[j])
        continue;
      const Real stoi = std::abs(_mgd.eqm_stoichiometry(j, basis_ind));
      if (stoi > best_stoi)
      {
        best_stoi = stoi;
        swap_into_basis = j;
        legitimate_swap_found = true;
      }
    }
    if (legitimate_swap_found)
      performSwapNoCheck(basis_ind, swap_into_basis);
    else
      mooseError("Attempting to change constraint of gas ",
                 _mgd.basis_species_name[basis_ind],
                 " to MOLES_BULK_SPECIES, which involves a search for a suitable non-gas species "
                 "to swap with.  No such species was found");
  }
  else
    changeConstraintToBulk(basis_ind, computeBulkFromMolalities(basis_ind));
}

void
GeochemicalSystem::changeConstraintToBulk(unsigned basis_ind, Real value)
{
  if (basis_ind >= _num_basis)
    mooseError("Cannot changeConstraintToBulk for species ",
               basis_ind,
               " because there are only ",
               _num_basis,
               " basis species");
  if (_mgd.basis_species_gas[basis_ind])
    mooseError("Attempting to changeConstraintToBulk for a gas species.  Since a swap is involved, "
               "you cannot specify a value for the bulk number of moles.  You must use "
               "changeConstraintToBulk(basis_ind) method instead of "
               "changeConstraintToBulk(basis_ind, value)");
  if (basis_ind == 0)
    _constraint_meaning[basis_ind] = ConstraintMeaningEnum::MOLES_BULK_WATER;
  else
    _constraint_meaning[basis_ind] = ConstraintMeaningEnum::MOLES_BULK_SPECIES;
  setConstraintValue(basis_ind, value);

  // it is possible that FIXED_ACTIVITY just became MOLES_BULK_SPECIES
  buildKnownBasisActivities(_basis_activity_known, _basis_activity);

  // it is likely the algebraic system has changed
  buildAlgebraicInfo(_in_algebraic_system,
                     _num_basis_in_algebraic_system,
                     _num_in_algebraic_system,
                     _algebraic_index,
                     _basis_index);
}

void
GeochemicalSystem::addToBulkMoles(unsigned basis_ind, Real value)
{
  if (basis_ind >= _num_basis)
    mooseError("Cannot addToBulkMoles for species ",
               basis_ind,
               " because there are only ",
               _num_basis,
               " basis species");
  if (!(_constraint_meaning[basis_ind] ==
            GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_WATER ||
        _constraint_meaning[basis_ind] ==
            GeochemicalSystem::ConstraintMeaningEnum::MOLES_BULK_SPECIES))
    return;
  setConstraintValue(basis_ind, _constraint_value[basis_ind] + value);
}

void
GeochemicalSystem::setConstraintValue(unsigned basis_ind, Real value)
{
  if (basis_ind >= _num_basis)
    mooseError("Cannot setConstraintValue for species ",
               basis_ind,
               " because there are only ",
               _num_basis,
               " basis species");
  _constraint_value[basis_ind] = value;
  _original_constraint_value[basis_ind] = value;
  switch (_constraint_meaning[basis_ind])
  {
    case ConstraintMeaningEnum::MOLES_BULK_SPECIES:
    case ConstraintMeaningEnum::MOLES_BULK_WATER:
    {
      alterSystemBecauseBulkChanged();
      break;
    }
    case ConstraintMeaningEnum::KG_SOLVENT_WATER:
    case ConstraintMeaningEnum::FREE_MOLALITY:
    case ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES:
    {
      // the changes resulting from this change are very similar to setAlgebraicVariables
      _basis_molality[basis_ind] = value;
      computeConsistentConfiguration();
      break;
    }
    case ConstraintMeaningEnum::FUGACITY:
    {
      // the changes resulting from this change are very similar to setAlgebraicVariables
      _basis_activity[basis_ind] = value;
      _basis_molality[basis_ind] = 0.0;
      computeConsistentConfiguration();
      break;
    }
    case ConstraintMeaningEnum::ACTIVITY:
    {
      // the changes resulting from this change are very similar to setAlgebraicVariables
      _basis_activity[basis_ind] = value;
      _basis_molality[basis_ind] = _basis_activity[basis_ind] / _basis_activity_coef[basis_ind];
      computeConsistentConfiguration();
      break;
    }
  }
}

void
GeochemicalSystem::alterSystemBecauseBulkChanged()
{
  // Altering the constraints on bulk number of moles impacts various other things.
  // Here, we follow the initialize() and computeConsistentConfiguration() methods, picking out
  // what might have changed.
  // Because the constraint meanings have changed, enforcing charge-neutrality might be easy
  enforceChargeBalanceIfSimple(_constraint_value, _bulk_moles_old);
  // Because the constraint values have changed, either through charge-neutrality or directly
  // through changing the constraint values, the bulk moles must be updated:
  for (unsigned i = 0; i < _num_basis; ++i)
    if (_constraint_meaning[i] == ConstraintMeaningEnum::MOLES_BULK_SPECIES ||
        _constraint_meaning[i] == ConstraintMeaningEnum::MOLES_BULK_WATER)
      _bulk_moles_old[i] = _constraint_value[i];
  // Because the bulk mineral moles might have changed, the free mineral moles might have
  // changed
  computeFreeMineralMoles(_basis_molality); // free mineral moles might be changed
}

const std::string &
GeochemicalSystem::getOriginalRedoxLHS() const
{
  return _original_redox_lhs;
}

void
GeochemicalSystem::setModelGeochemicalDatabase(const ModelGeochemicalDatabase & mgd)
{
  _mgd = mgd;
}

const GeochemistrySpeciesSwapper &
GeochemicalSystem::getSwapper() const
{
  return _swapper;
}

void
GeochemicalSystem::setMineralRelatedFreeMoles(Real value)
{
  // mole numbers of basis minerals
  for (unsigned i = 0; i < _num_basis; ++i)
    if (_mgd.basis_species_mineral[i])
      _basis_molality[i] = value;

  // mole numbers of sorption sites
  for (const auto & name_info :
       _mgd.surface_complexation_info) // all minerals involved in surface complexation
    for (const auto & name_frac :
         name_info.second.sorption_sites) // all sorption sites on the given mineral
    {
      if (_mgd.basis_species_index.count(name_frac.first))
        _basis_molality[_mgd.basis_species_index.at(name_frac.first)] = value;
      else
        _eqm_molality[_mgd.eqm_species_index.at(name_frac.first)] = value;
    }

  // mole numbers of sorbed equilibrium species
  for (unsigned j = 0; j < _num_eqm; ++j)
  {
    if (_mgd.eqm_species_mineral[j])
      _eqm_molality[j] = 0.0; // Note: explicitly setting to zero, irrespective of user input,
                              // to ensure consistency with the rest of the code
    if (_mgd.surface_sorption_related[j])
      _eqm_molality[j] = value;
  }

  // mole numbers of kinetic species
  for (unsigned k = 0; k < _num_kin; ++k)
    if (_mgd.kin_species_mineral[k])
      _kin_moles[k] = value;
}

Real
GeochemicalSystem::getEquilibriumActivity(unsigned eqm_ind) const
{
  if (eqm_ind >= _num_eqm)
    mooseError("Cannot retrieve activity for equilibrium species ",
               eqm_ind,
               " since there are only ",
               _num_eqm,
               " equilibrium species");
  if (_mgd.eqm_species_mineral[eqm_ind])
    return 1.0;
  else if (_mgd.eqm_species_gas[eqm_ind])
  {
    Real log10f = 0.0;
    for (unsigned basis_i = 0; basis_i < _num_basis; ++basis_i)
      log10f += _mgd.eqm_stoichiometry(eqm_ind, basis_i) * std::log10(_basis_activity[basis_i]);
    log10f -= getLog10K(eqm_ind);
    return std::pow(10.0, log10f);
  }
  else
    return _eqm_activity_coef[eqm_ind] * _eqm_molality[eqm_ind];
}

const std::vector<Real> &
GeochemicalSystem::computeAndGetEquilibriumActivity()
{
  for (unsigned j = 0; j < _num_eqm; ++j)
    _eqm_activity[j] = getEquilibriumActivity(j);
  return _eqm_activity;
}

void
GeochemicalSystem::updateOldWithCurrent(const DenseVector<Real> & mole_additions)
{
  if (mole_additions.size() != _num_basis + _num_kin)
    mooseError("The increment in mole numbers (mole_additions) needs to be of size ",
               _num_basis,
               " + ",
               _num_kin,
               " but it is of size ",
               mole_additions.size());

  // Copy the current kin_moles to the old values
  for (unsigned k = 0; k < _num_kin; ++k)
    _kin_moles_old[k] = _kin_moles[k];

  // The following:
  // - just returns if constraint is not BULK type.
  // - ensures charge balance if simple
  // - sets the constraint value and bulk_moles_old (for BULK-type species)
  // - computes basis_molality for the BULK-type mineral species
  for (unsigned i = 0; i < _num_basis; ++i)
    addToBulkMoles(i, mole_additions(i));

  // The following:
  // - sets bulk_moles_old to the Constraint value for BULK-type species (duplicating the above
  // function)
  // - for the other species, computes bulk_moles_old from the molalities
  computeBulk(_bulk_moles_old);

  // It is possible that the user would also like to enforceChargeBalance() now, and that is fine
  // - there will be no negative consequences
}

void
GeochemicalSystem::addKineticRates(Real dt,
                                   DenseVector<Real> & mole_additions,
                                   DenseMatrix<Real> & dmole_additions)
{
  if (_num_kin == 0)
    return;

  // check sizes
  const unsigned tot = mole_additions.size();
  if (!(tot == _num_kin + _num_basis && dmole_additions.m() == tot && dmole_additions.n() == tot))
    mooseError("addKineticRates: incorrectly sized additions: ",
               tot,
               " ",
               dmole_additions.m(),
               " ",
               dmole_additions.n());

  // construct eqm_activity for species that we need
  for (unsigned j = 0; j < _num_eqm; ++j)
    if (_mgd.eqm_species_gas[j] || _mgd.eqm_species_name[j] == "H+" ||
        _mgd.eqm_species_name[j] == "OH-")
      _eqm_activity[j] = getEquilibriumActivity(j);

  // calculate the rates and put into appropriate slots
  Real rate;
  Real drate_dkin;
  std::vector<Real> drate_dmol(_num_basis);
  for (const auto & krd : _mgd.kin_rate)
  {
    const unsigned kin = krd.kinetic_species_index;
    GeochemistryKineticRateCalculator::calculateRate(krd.promoting_indices,
                                                     krd.promoting_monod_indices,
                                                     krd.promoting_half_saturation,
                                                     krd.description,
                                                     _mgd.basis_species_name,
                                                     _mgd.basis_species_gas,
                                                     _basis_molality,
                                                     _basis_activity,
                                                     _basis_activity_known,
                                                     _mgd.eqm_species_name,
                                                     _mgd.eqm_species_gas,
                                                     _eqm_molality,
                                                     _eqm_activity,
                                                     _mgd.eqm_stoichiometry,
                                                     _kin_moles[kin],
                                                     _mgd.kin_species_molecular_weight[kin],
                                                     _kin_log10K[kin],
                                                     log10KineticActivityProduct(kin),
                                                     _mgd.kin_stoichiometry,
                                                     kin,
                                                     _temperature,
                                                     rate,
                                                     drate_dkin,
                                                     drate_dmol);

    /* the following block of code may be confusing at first sight.
     * (1) The usual case is that the kinetic reaction is written
     * kinetic -> basis_species, and direction = BOTH, and kinetic_bio_efficiency = -1
     * In this case, the following does mole_additions(kinetic_species) += -1 * rate * dt
     * If rate > 0 then the kinetic species decreases in mass.
     * The solver will then detect that the kinetic_species has changed, and adjust basis_species
     * accordingly, viz it will add rate * dt of basis species to the aqueous solution.
     * (2) The common biologically-catalysed case is the reaction is written
     * reactants -> products, catalysed by microbe.  In the database file this is written as
     * microbe -> -reactants + products
     * The input file will set direction = BOTH, and kinetic_bio_efficiency != -1
     * Suppose that rate > 0, corresponding to reactants being converted to products.
     * With kinetic_bio_efficiency = -1) this would correspond to microbe decreasing in mass,
     * however with kinetic_bio_efficiency > 0, this is not true.
     * The following block of code sets
     * mole_additions(microbe) = kinetic_bio_efficiency * rate * dt > 0
     * that is, the microbe mass is increasing.
     * However, this means the solver will decrease products and increase reactants,
     * by kinetic_bio_efficiency * rate * dt because it sees the reaction microbe -> -reactants +
     * products in the database file.
     * This is clearly wrong, because we actually want the end result to be that the products have
     * increased by rate * dt, and the reactants to have decreased by rate * dt.
     * So, to counter the solver, we add (-1 - kinetic_bio_efficiency) * rate * dt of reactants and
     * add (1 + kinetic_bio_efficienty) * rate * dt of products.
     * (3) The other situation is the biological death, where direction = DEATH.
     * In this case the database file contains microbe -> -reactants + products
     * However, independent of rate, we don't want to change the molality of reactants or products.
     * Following the logic of (2), to counter the solver, we add -kinetic_bio_efficiency * rate * dt
     * of reactants and kinetic_bio_efficiency * rate * dt of products
     */
    const unsigned ind = kin + _num_basis;
    mole_additions(ind) += krd.description.kinetic_bio_efficiency * rate * dt;
    dmole_additions(ind, ind) += krd.description.kinetic_bio_efficiency * drate_dkin * dt;
    const Real extra_additions = (krd.description.direction == DirectionChoiceEnum::DEATH)
                                     ? krd.description.kinetic_bio_efficiency
                                     : krd.description.kinetic_bio_efficiency + 1.0;
    for (unsigned i = 0; i < _num_basis; ++i)
    {
      dmole_additions(ind, i) += krd.description.kinetic_bio_efficiency * drate_dmol[i] * dt;
      const Real stoi_fac = _mgd.kin_stoichiometry(kin, i) * extra_additions * dt;
      mole_additions(i) += stoi_fac * rate;
      dmole_additions(i, ind) += stoi_fac * drate_dkin;
      for (unsigned j = 0; j < _num_basis; ++j)
        dmole_additions(i, j) += stoi_fac * drate_dmol[j];
    }

    const Real eff = krd.description.progeny_efficiency;
    if (eff != 0.0)
    {
      const unsigned bio_i = krd.progeny_index;
      if (bio_i < _num_basis)
      {
        mole_additions(bio_i) += eff * rate * dt;
        dmole_additions(bio_i, ind) += eff * drate_dkin * dt;
        for (unsigned i = 0; i < _num_basis; ++i)
          dmole_additions(bio_i, i) += eff * drate_dmol[i] * dt;
      }
      else
      {
        for (unsigned i = 0; i < _num_basis; ++i)
        {
          mole_additions(i) += _mgd.eqm_stoichiometry(bio_i - _num_basis, i) * eff * rate * dt;
          dmole_additions(i, ind) +=
              _mgd.eqm_stoichiometry(bio_i - _num_basis, i) * eff * drate_dkin * dt;
          for (unsigned j = 0; j < _num_basis; ++j)
            dmole_additions(i, j) +=
                _mgd.eqm_stoichiometry(bio_i - _num_basis, i) * eff * drate_dmol[j] * dt;
        }
      }
    }
  }
}
