//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EquilibriumGeochemicalSystem.h"
#include "GeochemistryActivityCalculators.h"

EquilibriumGeochemicalSystem::EquilibriumGeochemicalSystem(
    ModelGeochemicalDatabase & mgd,
    GeochemistryActivityCoefficients & gac,
    const GeochemistryIonicStrength & is,
    GeochemistrySpeciesSwapper & swapper,
    const std::vector<std::string> & swap_out_of_basis,
    const std::vector<std::string> & swap_into_basis,
    const std::string & charge_balance_species,
    const std::vector<std::string> & constrained_species,
    const std::vector<Real> & constraint_value,
    const MultiMooseEnum & constraint_meaning,
    Real initial_temperature,
    unsigned iters_to_make_consistent,
    Real min_initial_molality)
  : _mgd(mgd),
    _num_basis(mgd.basis_species_index.size()),
    _num_eqm(mgd.eqm_species_index.size()),
    _num_redox(mgd.redox_stoichiometry.m()),
    _num_surface_pot(mgd.surface_sorption_name.size()),
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
    _constraint_meaning(constraint_meaning.size()),
    _eqm_log10K(_num_eqm),
    _redox_log10K(_num_redox),
    _num_basis_in_algebraic_system(0),
    _num_in_algebraic_system(0),
    _in_algebraic_system(_num_basis),
    _algebraic_index(_num_basis),
    _basis_index(_num_basis),
    _bulk_moles(_num_basis),
    _basis_molality(_num_basis),
    _basis_activity_known(_num_basis),
    _basis_activity(_num_basis),
    _eqm_molality(_num_eqm),
    _basis_activity_coef(_num_basis),
    _eqm_activity_coef(_num_eqm),
    _surface_pot_expr(_num_surface_pot),
    _sorbing_surface_area(_num_surface_pot),
    _iters_to_make_consistent(iters_to_make_consistent),
    _temperature(initial_temperature),
    _min_initial_molality(min_initial_molality)
{
  for (unsigned i = 0; i < constraint_meaning.size(); ++i)
    _constraint_meaning[i] = static_cast<ConstraintMeaningEnum>(constraint_meaning.get(i));
  checkAndInitialize();
}

EquilibriumGeochemicalSystem::EquilibriumGeochemicalSystem(
    ModelGeochemicalDatabase & mgd,
    GeochemistryActivityCoefficients & gac,
    const GeochemistryIonicStrength & is,
    GeochemistrySpeciesSwapper & swapper,
    const std::vector<std::string> & swap_out_of_basis,
    const std::vector<std::string> & swap_into_basis,
    const std::string & charge_balance_species,
    const std::vector<std::string> & constrained_species,
    const std::vector<Real> & constraint_value,
    const std::vector<ConstraintMeaningEnum> & constraint_meaning,
    Real initial_temperature,
    unsigned iters_to_make_consistent,
    Real min_initial_molality)
  : _mgd(mgd),
    _num_basis(mgd.basis_species_index.size()),
    _num_eqm(mgd.eqm_species_index.size()),
    _num_redox(mgd.redox_stoichiometry.m()),
    _num_surface_pot(mgd.surface_sorption_name.size()),
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
    _constraint_meaning(constraint_meaning),
    _eqm_log10K(_num_eqm),
    _redox_log10K(_num_redox),
    _num_basis_in_algebraic_system(0),
    _num_in_algebraic_system(0),
    _in_algebraic_system(_num_basis),
    _algebraic_index(_num_basis),
    _basis_index(_num_basis),
    _bulk_moles(_num_basis),
    _basis_molality(_num_basis),
    _basis_activity_known(_num_basis),
    _basis_activity(_num_basis),
    _eqm_molality(_num_eqm),
    _basis_activity_coef(_num_basis),
    _eqm_activity_coef(_num_eqm),
    _surface_pot_expr(_num_surface_pot),
    _sorbing_surface_area(_num_surface_pot),
    _iters_to_make_consistent(iters_to_make_consistent),
    _temperature(initial_temperature),
    _min_initial_molality(min_initial_molality)
{
  checkAndInitialize();
}

void
EquilibriumGeochemicalSystem::checkAndInitialize()
{
  if (_mgd.kin_species_name.size() != 0)
    mooseError("Equilibrium geochemical systems cannot use models that include kinetic species");
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
  if (_constrained_species.size() != _constraint_meaning.size())
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
  std::vector<ConstraintMeaningEnum> c_m(_num_basis);
  for (unsigned i = 0; i < _num_basis; ++i)
  {
    const unsigned basis_ind = _mgd.basis_species_index.at(_constrained_species[i]);
    c_s[basis_ind] = _constrained_species[i];
    c_v[basis_ind] = _constraint_value[i];
    c_m[basis_ind] = _constraint_meaning[i];
  }
  _constrained_species = c_s;
  _constraint_value = c_v;
  _original_constraint_value = c_v;
  _constraint_meaning = c_m;

  // run through the constraints, checking physical and chemical consistency
  for (unsigned i = 0; i < _constrained_species.size(); ++i)
  {
    const std::string name = _constrained_species[i];

    // if the mass of solvent water is provided, check it is positive
    if (_constraint_meaning[i] == ConstraintMeaningEnum::KG_SOLVENT_WATER)
      if (_constraint_value[i] <= 0.0)
        mooseError("Specified mass of solvent water must be positive: you entered ",
                   _constraint_value[i]);

    // if activity is provided, check it is positive
    if (_constraint_meaning[i] == ConstraintMeaningEnum::ACTIVITY)
      if (_constraint_value[i] <= 0.0)
        mooseError("Specified activity values must be positive: you entered ",
                   _constraint_value[i]);

    // if fugacity is provided, check it is positive
    if (_constraint_meaning[i] == ConstraintMeaningEnum::FUGACITY)
      if (_constraint_value[i] <= 0.0)
        mooseError("Specified fugacity values must be positive: you entered ",
                   _constraint_value[i]);

    // if free molality is provided, check it is positive
    if (_constraint_meaning[i] == ConstraintMeaningEnum::FREE_MOLALITY)
      if (_constraint_value[i] <= 0.0)
        mooseError("Specified free molality values must be positive: you entered ",
                   _constraint_value[i]);

    // if free moles of a mineral is provided, check it is positive
    if (_constraint_meaning[i] == ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES)
      if (_constraint_value[i] <= 0.0)
        mooseError("Specified free mole number of mineral species must be positive: you entered ",
                   _constraint_value[i]);

    // check that water is provided with correct meaning
    if (name == "H2O")
      if (!(_constraint_meaning[i] == ConstraintMeaningEnum::MOLES_BULK_WATER ||
            _constraint_meaning[i] == ConstraintMeaningEnum::KG_SOLVENT_WATER ||
            _constraint_meaning[i] == ConstraintMeaningEnum::ACTIVITY))
        mooseError("H2O must be provided with either a mass of solvent water, a bulk number of "
                   "moles, or an activity");

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
                   " must be provided with a free number of moles or a bulk number of moles");

    // check that non-water, non-minerals, non-gases are provided with the correct meaning
    if (name != "H2O" && !_mgd.basis_species_gas[i] && !_mgd.basis_species_mineral[i])
      if (!(_constraint_meaning[i] == ConstraintMeaningEnum::FREE_MOLALITY ||
            _constraint_meaning[i] == ConstraintMeaningEnum::ACTIVITY ||
            _constraint_meaning[i] == ConstraintMeaningEnum::MOLES_BULK_SPECIES))
        mooseError("The basis species ",
                   name,
                   " must be provided with a free molality, bulk number of moles, or an activity");

    // check that the charge-balance species has been provided MOLES_BULK_SPECIES
    if (name == _charge_balance_species)
      if (_constraint_meaning[i] != ConstraintMeaningEnum::MOLES_BULK_SPECIES)
        mooseError("For code consistency, the species ",
                   name,
                   " must be provided with a bulk number of moles because it is the charge-balance "
                   "species.  The value provided should be a reasonable estimate of the mole "
                   "number, but will be overridden as the solve progresses");
  }
  initialize();
}

void
EquilibriumGeochemicalSystem::initialize()
{
  buildTemperatureDependentQuantities(_temperature);
  enforceChargeBalanceIfSimple(_constraint_value);
  buildAlgebraicInfo(_in_algebraic_system,
                     _num_basis_in_algebraic_system,
                     _num_in_algebraic_system,
                     _algebraic_index,
                     _basis_index);
  initBulkAndFree(_bulk_moles, _basis_molality);
  buildKnownBasisActivities(_basis_activity_known, _basis_activity);

  _eqm_molality.assign(_num_eqm, 0.0);
  _surface_pot_expr.assign(_num_surface_pot, 1.0);

  computeConsistentConfiguration();
}

void
EquilibriumGeochemicalSystem::computeConsistentConfiguration()
{
  // the steps 1 and 2 below could be iterated for a long time (or a Newton process could even be
  // followed) to provide better estimates of activities and molalities, but this is not done in the
  // conventional geochemistry approach: there are just too many unknowns and approximations
  // employed during the algebraic-system solve to justify iterating towards the perfectly
  // consistent initial condition
  for (unsigned picard = 0; picard < _iters_to_make_consistent + 1; ++picard)
  {
    // Step 1: compute ionic strengths and activities using the eqm molalities
    _gac.setInternalParameters(_temperature, _mgd, _basis_molality, _eqm_molality, {});
    _gac.buildActivityCoefficients(_mgd, _basis_activity_coef, _eqm_activity_coef);
    updateBasisMolalityForKnownActivity(_basis_molality);
    computeRemainingBasisActivities(_basis_activity);

    // Step 2: compute equilibrium molality based on the activities just computed
    computeEqmMolalities(_eqm_molality);
  }

  computeBulk(_bulk_moles);
  computeFreeMineralMoles(_basis_molality);
  computeSorbingSurfaceArea(_sorbing_surface_area);
}

unsigned
EquilibriumGeochemicalSystem::getChargeBalanceBasisIndex() const
{
  return _charge_balance_basis_index;
}

Real
EquilibriumGeochemicalSystem::getLog10K(unsigned j) const
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
EquilibriumGeochemicalSystem::getNumRedox() const
{
  return _num_redox;
}

Real
EquilibriumGeochemicalSystem::getRedoxLog10K(unsigned red) const
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
EquilibriumGeochemicalSystem::log10RedoxActivityProduct(unsigned red) const
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

void EquilibriumGeochemicalSystem::buildTemperatureDependentQuantities(Real /*temperature*/)
{
  // When temperature least-squares stuff has been done, this function will be altered
  // at 25degC currently
  for (unsigned eqm_j = 0; eqm_j < _num_eqm; ++eqm_j)
    _eqm_log10K[eqm_j] = _mgd.eqm_log10K(eqm_j, 1);
  for (unsigned red = 0; red < _num_redox; ++red)
    _redox_log10K[red] = _mgd.redox_log10K(red, 1);
}

void
EquilibriumGeochemicalSystem::buildAlgebraicInfo(std::vector<bool> & in_algebraic_system,
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

  num_in_algebraic_system = num_basis_in_algebraic_system + _num_surface_pot;
}

unsigned
EquilibriumGeochemicalSystem::getNumInAlgebraicSystem() const
{
  return _num_in_algebraic_system;
}

unsigned
EquilibriumGeochemicalSystem::getNumBasisInAlgebraicSystem() const
{
  return _num_basis_in_algebraic_system;
}

unsigned
EquilibriumGeochemicalSystem::getNumSurfacePotentials() const
{
  return _num_surface_pot;
}

const std::vector<bool> &
EquilibriumGeochemicalSystem::getInAlgebraicSystem() const
{
  return _in_algebraic_system;
}

const std::vector<unsigned> &
EquilibriumGeochemicalSystem::getBasisIndexOfAlgebraicSystem() const
{
  return _basis_index;
}

const std::vector<unsigned> &
EquilibriumGeochemicalSystem::getAlgebraicIndexOfBasisSystem() const
{
  return _algebraic_index;
}

std::vector<Real>
EquilibriumGeochemicalSystem::getAlgebraicVariableValues() const
{
  std::vector<Real> var(_num_basis_in_algebraic_system + _num_surface_pot);
  for (unsigned a = 0; a < _num_basis_in_algebraic_system; ++a)
    var[a] = _basis_molality[_basis_index[a]];
  for (unsigned s = 0; s < _num_surface_pot; ++s)
    var[s + _num_basis_in_algebraic_system] = _surface_pot_expr[s];
  return var;
}

std::vector<Real>
EquilibriumGeochemicalSystem::getAlgebraicBasisValues() const
{
  std::vector<Real> var(_num_basis_in_algebraic_system);
  for (unsigned a = 0; a < _num_basis_in_algebraic_system; ++a)
    var[a] = _basis_molality[_basis_index[a]];
  return var;
}

DenseVector<Real>
EquilibriumGeochemicalSystem::getAlgebraicVariableDenseValues() const
{
  DenseVector<Real> var(_num_in_algebraic_system);
  for (unsigned a = 0; a < _num_basis_in_algebraic_system; ++a)
    var(a) = _basis_molality[_basis_index[a]];
  for (unsigned s = 0; s < _num_surface_pot; ++s)
    var(s + _num_basis_in_algebraic_system) = _surface_pot_expr[s];
  return var;
}

void
EquilibriumGeochemicalSystem::initBulkAndFree(std::vector<Real> & bulk_moles,
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
        bulk_moles[i] = value;
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
        bulk_moles[i] = value * GeochemistryConstants::MOLES_PER_KG_WATER /
                        0.999;     // initial guess (water is not an algebraic variable).  Will be
                                   // determined exactly during the solve
        basis_molality[i] = value; // mass of solvent water
        break;
      }
      case ConstraintMeaningEnum::MOLES_BULK_SPECIES:
      {
        bulk_moles[i] = value;
        basis_molality[i] = std::max(
            _min_initial_molality,
            0.9 * value / basis_molality[0]); // initial guess (i is an algebraic variable).  This
                                              // is what we solve for in the Newton process
        break;
      }
      case ConstraintMeaningEnum::FREE_MOLALITY:
      {
        bulk_moles[i] =
            value * basis_molality[0] / 0.9; // initial guess (i is not an algebraic variable). Will
                                             // be determined exactly during the solve
        basis_molality[i] = value;
        break;
      }
      case ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES:
      {
        bulk_moles[i] = value / 0.9; // initial guess (i is not an algebraic variable).  Will be
                                     // determined exactly during the solve
        basis_molality[i] = value;   // note, this is *moles*, not molality
        break;
      }
      case ConstraintMeaningEnum::FUGACITY:
      {
        bulk_moles[i] = 0.0; // initial guess (i is not an algebraic variable).  will be determined
                             // exactly after the solve
        basis_molality[i] = value; // never used since this is a gas
        break;
      }
      case ConstraintMeaningEnum::ACTIVITY:
      {
        bulk_moles[i] = value / 0.9; // initial guess (i is not an algebraic variable).  Will be
                                     // determined exactly during the solve
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
EquilibriumGeochemicalSystem::getSolventWaterMass() const
{
  return _basis_molality[0];
}

const std::vector<Real> &
EquilibriumGeochemicalSystem::getBulkMoles() const
{
  return _bulk_moles;
}

const std::vector<Real> &
EquilibriumGeochemicalSystem::getSolventMassAndFreeMolalityAndMineralMoles() const
{
  return _basis_molality;
}

void
EquilibriumGeochemicalSystem::buildKnownBasisActivities(std::vector<bool> & basis_activity_known,
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
EquilibriumGeochemicalSystem::getBasisActivityKnown() const
{
  return _basis_activity_known;
}

Real
EquilibriumGeochemicalSystem::getBasisActivity(unsigned i) const
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
EquilibriumGeochemicalSystem::getBasisActivity() const
{
  return _basis_activity;
}

Real
EquilibriumGeochemicalSystem::getEquilibriumMolality(unsigned j) const
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
EquilibriumGeochemicalSystem::getEquilibriumMolality() const
{
  return _eqm_molality;
}

void
EquilibriumGeochemicalSystem::computeRemainingBasisActivities(
    std::vector<Real> & basis_activity) const
{
  if (!_basis_activity_known[0])
    basis_activity[0] = _gac.waterActivity();
  for (unsigned basis_ind = 1; basis_ind < _num_basis; ++basis_ind) // don't loop over water
    if (!_basis_activity_known[basis_ind]) // minerals, gases and species with activities provided
                                           // by the user
      basis_activity[basis_ind] = _basis_activity_coef[basis_ind] * _basis_molality[basis_ind];
}

Real
EquilibriumGeochemicalSystem::getEquilibriumActivityCoefficient(unsigned j) const
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
EquilibriumGeochemicalSystem::getEquilibriumActivityCoefficient() const
{
  return _eqm_activity_coef;
}

Real
EquilibriumGeochemicalSystem::getBasisActivityCoefficient(unsigned i) const
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
EquilibriumGeochemicalSystem::getBasisActivityCoefficient() const
{
  return _basis_activity_coef;
}

void
EquilibriumGeochemicalSystem::updateBasisMolalityForKnownActivity(
    std::vector<Real> & basis_molality) const
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
EquilibriumGeochemicalSystem::log10ActivityProduct(unsigned eqm_j) const
{
  Real log10ap = 0.0;
  for (unsigned basis_i = 0; basis_i < _num_basis; ++basis_i)
    log10ap += _mgd.eqm_stoichiometry(eqm_j, basis_i) * std::log10(_basis_activity[basis_i]);
  return log10ap;
}

void
EquilibriumGeochemicalSystem::computeEqmMolalities(std::vector<Real> & eqm_molality) const
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
EquilibriumGeochemicalSystem::surfaceSorptionModifier(unsigned eqm_j) const
{
  if (eqm_j >= _num_eqm)
    return 1.0;
  if (!_mgd.surface_sorption_related[eqm_j])
    return 1.0;
  return std::pow(_surface_pot_expr[_mgd.surface_sorption_number[eqm_j]],
                  2.0 * _mgd.eqm_species_charge[eqm_j]);
}

void
EquilibriumGeochemicalSystem::enforceChargeBalanceIfSimple(
    std::vector<Real> & constraint_value) const
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
  // all charged basis species must have been provided with a MOLES_BULK_SPECIES value, so we can
  // easily enforce charge neutrality
  tot_charge -= _mgd.basis_species_charge[_charge_balance_basis_index] *
                constraint_value[_charge_balance_basis_index];
  constraint_value[_charge_balance_basis_index] =
      -tot_charge / _mgd.basis_species_charge[_charge_balance_basis_index];
}

Real
EquilibriumGeochemicalSystem::getTotalCharge() const
{
  Real tot_charge = 0.0;
  for (unsigned basis_i = 0; basis_i < _num_basis; ++basis_i)
    tot_charge += _mgd.basis_species_charge[basis_i] * _bulk_moles[basis_i];
  return tot_charge;
}

void
EquilibriumGeochemicalSystem::enforceChargeBalance()
{
  enforceChargeBalance(_constraint_value, _bulk_moles);
}

void
EquilibriumGeochemicalSystem::enforceChargeBalance(std::vector<Real> & constraint_value,
                                                   std::vector<Real> & bulk_moles) const
{
  const Real tot_charge = getTotalCharge();
  constraint_value[_charge_balance_basis_index] -=
      tot_charge / _mgd.basis_species_charge[_charge_balance_basis_index];
  bulk_moles[_charge_balance_basis_index] = constraint_value[_charge_balance_basis_index];
}

void
EquilibriumGeochemicalSystem::setAlgebraicVariables(const DenseVector<Real> & algebraic_var)
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

  computeConsistentConfiguration();
}

void
EquilibriumGeochemicalSystem::computeBulk(std::vector<Real> & bulk_moles) const
{
  const Real nw = _basis_molality[0];
  for (unsigned i = 0; i < _num_basis; ++i)
  {
    const Real value = _constraint_value[i];
    const ConstraintMeaningEnum meaning = _constraint_meaning[i];
    switch (meaning)
    {
      case ConstraintMeaningEnum::MOLES_BULK_WATER:
      {
        bulk_moles[i] = value;
        break;
      }
      case ConstraintMeaningEnum::KG_SOLVENT_WATER:
      {
        bulk_moles[i] = GeochemistryConstants::MOLES_PER_KG_WATER;
        for (unsigned j = 0; j < _num_eqm; ++j)
          bulk_moles[i] += _mgd.eqm_stoichiometry(j, i) * _eqm_molality[j];
        bulk_moles[i] *= nw;
        break;
      }
      case ConstraintMeaningEnum::MOLES_BULK_SPECIES:
      {
        bulk_moles[i] = value;
        break;
      }
      case ConstraintMeaningEnum::FREE_MOLALITY:
      {
        bulk_moles[i] = _basis_molality[i];
        for (unsigned j = 0; j < _num_eqm; ++j)
          bulk_moles[i] += _mgd.eqm_stoichiometry(j, i) * _eqm_molality[j];
        bulk_moles[i] *= nw;
        break;
      }
      case ConstraintMeaningEnum::FREE_MOLES_MINERAL_SPECIES:
      {
        bulk_moles[i] = value;
        for (unsigned j = 0; j < _num_eqm; ++j)
          bulk_moles[i] += nw * _mgd.eqm_stoichiometry(j, i) * _eqm_molality[j];
        break;
      }
      case ConstraintMeaningEnum::FUGACITY:
      {
        bulk_moles[i] = 0.0;
        for (unsigned j = 0; j < _num_eqm; ++j)
          bulk_moles[i] += nw * _mgd.eqm_stoichiometry(j, i) * _eqm_molality[j];
        break;
      }
      case ConstraintMeaningEnum::ACTIVITY:
      {
        bulk_moles[i] = _basis_molality[i];
        for (unsigned j = 0; j < _num_eqm; ++j)
          bulk_moles[i] += _mgd.eqm_stoichiometry(j, i) * _eqm_molality[j];
        bulk_moles[i] *= nw;
        break;
      }
    }
  }
}

Real
EquilibriumGeochemicalSystem::getResidualComponent(unsigned algebraic_ind) const
{
  if (algebraic_ind >= _num_in_algebraic_system)
    mooseError("Cannot retrieve residual for algebraic index ",
               algebraic_ind,
               " because there are only ",
               _num_basis_in_algebraic_system,
               " molalities in the algebraic system and ",
               _num_surface_pot,
               " surface potentials");

  if (algebraic_ind < _num_basis_in_algebraic_system) // residual for basis molality
  {
    const unsigned basis_i = _basis_index[algebraic_ind];
    Real res = 0.0;
    if (basis_i == 0)
      res += -_bulk_moles[basis_i] + _basis_molality[0] * GeochemistryConstants::MOLES_PER_KG_WATER;
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
        else // _bulk_moles has been computed using computeBulk, either from constructor,
             // setAlgebraicVariables or performSwap
          res += _mgd.basis_species_charge[i] * _bulk_moles[i] / _mgd.basis_species_charge[basis_i];
      }
    }
    else
      res += -_bulk_moles[basis_i] + _basis_molality[0] * _basis_molality[basis_i];
    for (unsigned eqm_j = 0; eqm_j < _num_eqm; ++eqm_j)
      res += _basis_molality[0] * _mgd.eqm_stoichiometry(eqm_j, basis_i) * _eqm_molality[eqm_j];
    return res;
  }

  // else: residual for surface potential
  const unsigned sp = algebraic_ind - _num_basis_in_algebraic_system;
  Real res = surfacePotPrefactor(sp) * (_surface_pot_expr[sp] - 1.0 / _surface_pot_expr[sp]);
  for (unsigned j = 0; j < _num_eqm; ++j)
    if (_mgd.surface_sorption_related[j] && _mgd.surface_sorption_number[j] == sp)
      res += _basis_molality[0] * _mgd.eqm_species_charge[j] * _eqm_molality[j];
  return res;
}

void
EquilibriumGeochemicalSystem::computeJacobian(const DenseVector<Real> & res,
                                              DenseMatrix<Real> & jac) const
{
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
      if (basis_of_b == 0) // special cases: mass of solvent water is an unknown
      {
        if (basis_of_a != _charge_balance_basis_index)
          jac(a, 0) += (res(a) + _bulk_moles[basis_of_a]) / nw;
        else
        {
          Real extra_term = 0.0;
          for (unsigned i = 0; i < _num_basis; ++i)
          {
            if (i == _charge_balance_basis_index)
              continue;
            else if (_mgd.basis_species_charge[i] ==
                     0.0) // certainly includes water, minerals and gases
              continue;
            else if (_in_algebraic_system[i]) // bulk moles must have been provided by user
              extra_term += _mgd.basis_species_charge[i] * _bulk_moles[i] /
                            _mgd.basis_species_charge[basis_of_a];
            // note that if species i has ACTIVITY or FREE_MOLALITY set then its bulk moles is
            // linear in nw so it should not appear in extra_term
          }
          jac(a, 0) += (res(a) - extra_term) / nw;
        }
      }
      else
      {
        if (a == b)
          jac(a, b) += nw;
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
            else if (_constraint_meaning[i] == ConstraintMeaningEnum::ACTIVITY ||
                     _constraint_meaning[i] == ConstraintMeaningEnum::FREE_MOLALITY)
            {
              const Real prefactor =
                  _mgd.basis_species_charge[i] * nw / _mgd.basis_species_charge[basis_of_a];
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
}

const ModelGeochemicalDatabase &
EquilibriumGeochemicalSystem::getModelGeochemicalDatabase() const
{
  return _mgd;
}

void
EquilibriumGeochemicalSystem::computeFreeMineralMoles(std::vector<Real> & basis_molality) const
{

  const Real nw = _basis_molality[0];
  for (unsigned i = 0; i < _num_basis; ++i)
    if (_mgd.basis_species_mineral[i])
    {
      basis_molality[i] = _bulk_moles[i];
      for (unsigned j = 0; j < _num_eqm; ++j)
        basis_molality[i] -= nw * _mgd.eqm_stoichiometry(j, i) * _eqm_molality[j];
    }
}

std::vector<Real>
EquilibriumGeochemicalSystem::getSaturationIndices() const
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
EquilibriumGeochemicalSystem::performSwap(unsigned swap_out_of_basis, unsigned swap_into_basis)
{
  if (swap_out_of_basis == 0)
    mooseError("EquilibriumGeochemicalSystem: attempting to swap out water and replace it by ",
               _mgd.eqm_species_name[swap_into_basis],
               ".  This could be because the algorithm would like to "
               "swap out the charge-balance species, in which case you should choose a "
               "different charge-balance species");
  if (swap_out_of_basis == _charge_balance_basis_index)
    mooseError("EquilibriumGeochemicalSystem: attempting to swap the charge-balance species out of "
               "the basis");
  if (_mgd.basis_species_gas[swap_out_of_basis])
    mooseError("EquilibriumGeochemicalSystem: attempting to swap a gas out of the basis");
  if (_mgd.eqm_species_gas[swap_into_basis])
    mooseError("EquilibriumGeochemicalSystem: attempting to swap a gas into the basis");

  // perform the swap
  DenseVector<Real> bm = _bulk_moles;
  _swapper.performSwap(_mgd, bm, swap_out_of_basis, swap_into_basis);

  // the swap_into_basis species now has fixed bulk moles irrespective of what the
  // swap_out_of_basis species had fixed
  _constraint_meaning[swap_out_of_basis] = ConstraintMeaningEnum::MOLES_BULK_SPECIES;

  // the bulk moles will have changed for many components.  The _bulk_moles values will be set in
  // computeConsistentConfiguration, below
  for (unsigned basis_i = 0; basis_i < _num_basis; ++basis_i)
    if (_constraint_meaning[basis_i] == ConstraintMeaningEnum::MOLES_BULK_SPECIES ||
        _constraint_meaning[basis_i] == ConstraintMeaningEnum::MOLES_BULK_WATER)
    {
      _constraint_value[basis_i] = bm(basis_i);
      _original_constraint_value[basis_i] = bm(basis_i);
    }

  // In the following, the molalities are carefully swapped so that the configuration stays
  // reasonably close to the configuration prior to the swap.  This is advantageous as it
  // initializes the Newton process with a better starting guess.
  Real molality_of_species_just_swapped_in =
      _eqm_molality[swap_into_basis]; // is positive, or zero iff (mineral or gas)
  if (_mgd.basis_species_mineral[swap_out_of_basis] || _mgd.basis_species_gas[swap_out_of_basis])
  {
    // the species just swapped in is a mineral or a gas, so its equilibium molality is undefined:
    // make a guess for its molality
    molality_of_species_just_swapped_in =
        std::max(_min_initial_molality, 0.9 * bm(swap_out_of_basis));
  }
  Real molality_of_species_just_swapped_out =
      _basis_molality[swap_out_of_basis]; // can be negative if a consumed mineral
  _basis_molality[swap_out_of_basis] = molality_of_species_just_swapped_in;
  _eqm_molality[swap_into_basis] =
      std::max(0.0, molality_of_species_just_swapped_out); // this gets recalculated in
                                                           // computeConsistentConfiguration, below

  // depending if minerals were swapped in or out of the basis, the known activity may have changed
  buildKnownBasisActivities(_basis_activity_known, _basis_activity);

  // the equilibrium constants will have changed due to the swap
  buildTemperatureDependentQuantities(_temperature);

  // due to re-orderings in mgd, the charge-balance basis index might have changed
  _charge_balance_basis_index = _mgd.basis_species_index.at(_charge_balance_species);
  // charge balance might be able to be performed easily
  enforceChargeBalanceIfSimple(_constraint_value);

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
EquilibriumGeochemicalSystem::getNumInBasis() const
{
  return _num_basis;
}

unsigned
EquilibriumGeochemicalSystem::getNumInEquilibrium() const
{
  return _num_eqm;
}

void
EquilibriumGeochemicalSystem::setChargeBalanceSpecies(unsigned new_charge_balance_index)
{
  // because the original mole number of the charge balance species may have been changed due to
  // enforcing charge balance:
  _constraint_value[_charge_balance_basis_index] =
      _original_constraint_value[_charge_balance_basis_index];
  _bulk_moles[_charge_balance_basis_index] = _constraint_value[_charge_balance_basis_index];
  // now change the charge balance info
  _charge_balance_basis_index = new_charge_balance_index;
  _charge_balance_species = _mgd.basis_species_name[_charge_balance_basis_index];
  // enforce charge-balance if easily possible
  enforceChargeBalanceIfSimple(_constraint_value);
  // update _bulk_moles
  _bulk_moles[_charge_balance_basis_index] = _constraint_value[_charge_balance_basis_index];
}

bool
EquilibriumGeochemicalSystem::alterChargeBalanceSpecies(Real threshold_molality)
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
EquilibriumGeochemicalSystem::revertToOriginalChargeBalanceSpecies()
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
EquilibriumGeochemicalSystem::getIonicStrength() const
{
  return _is.ionicStrength(_mgd, _basis_molality, _eqm_molality, {});
}

Real
EquilibriumGeochemicalSystem::getStoichiometricIonicStrength() const
{
  return _is.stoichiometricIonicStrength(_mgd, _basis_molality, _eqm_molality, {});
}

Real
EquilibriumGeochemicalSystem::surfacePotPrefactor(unsigned sp) const
{
  return 0.5 * _sorbing_surface_area[sp] / GeochemistryConstants::FARADAY *
         std::sqrt(GeochemistryConstants::GAS_CONSTANT *
                   (_temperature + GeochemistryConstants::CELSIUS_TO_KELVIN) *
                   GeochemistryConstants::PERMITTIVITY_FREE_SPACE *
                   GeochemistryConstants::DIELECTRIC_CONSTANT_WATER *
                   GeochemistryConstants::DENSITY_WATER *
                   _is.ionicStrength(_mgd, _basis_molality, _eqm_molality, {}));
}

Real
EquilibriumGeochemicalSystem::getSurfacePotential(unsigned sp) const
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
EquilibriumGeochemicalSystem::getSurfaceCharge(unsigned sp) const
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
EquilibriumGeochemicalSystem::computeSorbingSurfaceArea(
    std::vector<Real> & sorbing_surface_area) const
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
EquilibriumGeochemicalSystem::getSorbingSurfaceArea() const
{
  return _sorbing_surface_area;
}
