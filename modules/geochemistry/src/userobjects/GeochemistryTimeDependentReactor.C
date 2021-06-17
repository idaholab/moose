//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryTimeDependentReactor.h"

registerMooseObject("GeochemistryApp", GeochemistryTimeDependentReactor);

InputParameters
GeochemistryTimeDependentReactor::sharedParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<unsigned>(
      "ramp_max_ionic_strength_subsequent",
      0,
      "The number of iterations over which to progressively increase the maximum ionic strength "
      "(from zero to max_ionic_strength) during time-stepping.  Unless a great deal occurs in each "
      "time step, this parameter can be set quite small");
  params.addCoupledVar(
      "mode",
      0.0,
      "This may vary temporally.  If mode=1 then 'dump' mode is used, which means all non-kinetic "
      "mineral masses are removed from the system before the equilibrium solution is sought (ie, "
      "removal occurs at the beginning of the time step).  If mode=2 then 'flow-through' mode is "
      "used, which means all mineral masses are removed from the system after it the "
      "equilbrium solution has been found (ie, at the end of a time step).  If mode=3 then 'flush' "
      "mode is used, then before the equilibrium solution is sought (ie, at the start of a time "
      "step) water+species is removed from the system at the same rate as pure water + non-mineral "
      "solutes are entering the system (specified in source_species_rates).  If mode=4 then "
      "'heat-exchanger' mode is used, which means the entire current aqueous solution is removed, "
      "then the source_species are added, then the temperature is set to 'cold_temperature', the "
      "system is solved and any precipitated minerals are removed, then the temperature is set to "
      "'temperature', the system re-solved and any precipitated minerals are removed.  If mode is "
      "any other number, no special mode is active (the system simply responds to the "
      "source_species_rates, controlled_activity_value, etc).");
  params.addCoupledVar(
      "temperature",
      25,
      "Temperature.  This has two different meanings if mode!=4.  (1) If no species are being "
      "added to the solution (no source_species_rates are positive) then this is the temperature "
      "of the aqueous solution.  (2) If species are being added, this is the temperature of the "
      "species being added.  In case (2), the final aqueous-solution temperature is computed "
      "assuming the species are added, temperature is equilibrated and then, if species are also "
      "being removed, they are removed.  If you wish to add species and simultaneously alter the "
      "temperature, you will have to use a sequence of heat-add-heat-add, etc steps.  In the case "
      "that mode=4, temperature is the final temperature of the aqueous solution");
  params.addCoupledVar(
      "cold_temperature",
      25,
      "This is only used if mode=4, where it is the cold temperature of the heat exchanger.");
  params.addRangeCheckedParam<unsigned>(
      "heating_increments",
      1,
      "heating_increments > 0",
      "This is only used if mode=4.  Internal to this object, the temperature is ramped from "
      "cold_temperature to temperature in heating_increments increments.  This helps difficult "
      "problems converge");
  params.addParam<Real>("initial_temperature",
                        25.0,
                        "The initial aqueous solution is equilibrated at this system before adding "
                        "reactants, changing temperature, etc.");
  params.addParam<Real>("close_system_at_time",
                        0.0,
                        "Time at which to 'close' the system, that is, change a kg_solvent_water "
                        "constraint to moles_bulk_water, and all free_molality and "
                        "free_moles_mineral_species to moles_bulk_species");
  params.addParam<std::vector<std::string>>(
      "remove_fixed_activity_name",
      "The name of the species that should have their activity or fugacity constraint removed at "
      "time given in remove_fixed_activity_time.  There should be an equal number of these names "
      "as times given in remove_fixed_activity_time.  Each of these must be in the basis and have "
      "an activity or fugacity constraint");
  params.addParam<std::vector<Real>>("remove_fixed_activity_time",
                                     "The times at which the species in remove_fixed_activity_name "
                                     "should have their activity or fugacity constraint removed.");
  params.addParam<std::vector<std::string>>(
      "source_species_names",
      "The name of the species that are added at rates given in source_species_rates.  There must "
      "be an equal number of these as source_species_rates.");
  params.addCoupledVar("source_species_rates",
                       "Rates, in mols/time_unit, of addition of the species with names given in "
                       "source_species_names.  A negative value corresponds to removing a species: "
                       "be careful that you don't cause negative mass problems!");
  params.addParam<std::vector<std::string>>(
      "controlled_activity_name",
      "The names of the species that have their activity or fugacity constrained.  There should be "
      "an equal number of these names as values given in controlled_activity_value.  NOTE: if "
      "these species are not in the basis, or they do not have an activity (or fugacity) "
      "constraint then their activity cannot be controlled: in this case MOOSE will ignore the "
      "value you prescribe in controlled_activity_value.");
  params.addCoupledVar("controlled_activity_value",
                       "Values of the activity or fugacity of the species in "
                       "controlled_activity_name list.  These should always be positive");
  params.addParam<bool>(
      "evaluate_kinetic_rates_always",
      true,
      "If true, then, evaluate the kinetic rates at every Newton step during the solve using the "
      "current values of molality, activity, etc (ie, implement an implicit solve).  If false, "
      "then evaluate the kinetic rates using the values of molality, activity, etc, at the start "
      "of the current time step (ie, implement an explicit solve)");
  params.addParam<std::vector<std::string>>(
      "kinetic_species_name",
      "Names of the kinetic species given initial values in kinetic_species_initial_value");
  params.addParam<std::vector<Real>>(
      "kinetic_species_initial_value",
      "Initial number of moles, mass or volume (depending on kinetic_species_unit) for each of the "
      "species named in kinetic_species_name");
  MultiMooseEnum kin_species_unit("dimensionless moles molal kg g mg ug kg_per_kg_solvent "
                                  "g_per_kg_solvent mg_per_kg_solvent ug_per_kg_solvent cm3");
  params.addParam<MultiMooseEnum>(
      "kinetic_species_unit",
      kin_species_unit,
      "Units of the numerical values given in kinetic_species_initial_value.  Moles: mole number.  "
      "kg: kilograms.  g: grams.  mg: milligrams.  ug: micrograms.  cm3: cubic centimeters");
  return params;
}

InputParameters
GeochemistryTimeDependentReactor::validParams()
{
  InputParameters params = GeochemistryReactorBase::validParams();
  params += GeochemistryTimeDependentReactor::sharedParams();
  params.addClassDescription("UserObject that controls the time-dependent geochemistry reaction "
                             "processes.  Spatial dependence is not possible using this class");
  return params;
}

GeochemistryTimeDependentReactor::GeochemistryTimeDependentReactor(
    const InputParameters & parameters)
  : GeochemistryReactorBase(parameters),
    _temperature(coupledValue("temperature")),
    _cold_temperature(coupledValue("cold_temperature")),
    _heating_increments(getParam<unsigned>("heating_increments")),
    _new_temperature(getParam<Real>("initial_temperature")),
    _previous_temperature(getParam<Real>("initial_temperature")),
    _egs(_mgd,
         _gac,
         _is,
         _swapper,
         getParam<std::vector<std::string>>("swap_out_of_basis"),
         getParam<std::vector<std::string>>("swap_into_basis"),
         getParam<std::string>("charge_balance_species"),
         getParam<std::vector<std::string>>("constraint_species"),
         getParam<std::vector<Real>>("constraint_value"),
         getParam<MultiMooseEnum>("constraint_unit"),
         getParam<MultiMooseEnum>("constraint_meaning"),
         _previous_temperature,
         getParam<unsigned>("extra_iterations_to_make_consistent"),
         getParam<Real>("min_initial_molality"),
         getParam<std::vector<std::string>>("kinetic_species_name"),
         getParam<std::vector<Real>>("kinetic_species_initial_value"),
         getParam<MultiMooseEnum>("kinetic_species_unit")),
    _solver(_mgd.basis_species_name.size(),
            _mgd.kin_species_name.size(),
            _is,
            getParam<Real>("abs_tol"),
            getParam<Real>("rel_tol"),
            getParam<unsigned>("max_iter"),
            getParam<Real>("max_initial_residual"),
            _small_molality,
            _max_swaps_allowed,
            getParam<std::vector<std::string>>("prevent_precipitation"),
            getParam<Real>("max_ionic_strength"),
            getParam<unsigned>("ramp_max_ionic_strength_initial"),
            getParam<bool>("evaluate_kinetic_rates_always")),
    _num_kin(_egs.getNumKinetic()),
    _close_system_at_time(getParam<Real>("close_system_at_time")),
    _closed_system(false),
    _source_species_names(getParam<std::vector<std::string>>("source_species_names")),
    _num_source_species(_source_species_names.size()),
    _source_species_rates(coupledValues("source_species_rates")),
    _remove_fixed_activity_name(getParam<std::vector<std::string>>("remove_fixed_activity_name")),
    _remove_fixed_activity_time(getParam<std::vector<Real>>("remove_fixed_activity_time")),
    _num_removed_fixed(_remove_fixed_activity_name.size()),
    _removed_fixed_activity(_num_removed_fixed, false),
    _controlled_activity_species_names(
        getParam<std::vector<std::string>>("controlled_activity_name")),
    _num_controlled_activity(_controlled_activity_species_names.size()),
    _controlled_activity_species_values(coupledValues("controlled_activity_value")),
    _mole_additions(_num_basis + _num_kin),
    _dmole_additions(_num_basis + _num_kin, _num_basis + _num_kin),
    _mode(coupledValue("mode")),
    _minerals_dumped(),
    _ramp_subsequent(getParam<unsigned>("ramp_max_ionic_strength_subsequent"))
{
  // check sources and set the rates
  if (coupledComponents("source_species_rates") != _num_source_species)
    paramError("source_species_names", "must have the same size as source_species_rates");
  for (unsigned i = 0; i < _num_source_species; ++i)
    if (!(_mgd.basis_species_index.count(_source_species_names[i]) == 1 ||
          _mgd.eqm_species_index.count(_source_species_names[i]) == 1 ||
          _mgd.kin_species_index.count(_source_species_names[i]) == 1))
      paramError("source_species_names",
                 "The name " + _source_species_names[i] +
                     " does not appear in the basis, equilibrium or kinetic species list");

  // check and set activity controllers
  for (unsigned i = 0; i < _num_removed_fixed; ++i)
  {
    if (_mgd.basis_species_index.count(_remove_fixed_activity_name[i]) == 0)
      paramError(
          "remove_fixed_activity_name",
          "The species ",
          _remove_fixed_activity_name[i],
          " is not in the basis, so cannot have its activity or fugacity constraint removed");
    else
    {
      const unsigned basis_ind = _mgd.basis_species_index.at(_remove_fixed_activity_name[i]);
      const GeochemicalSystem::ConstraintMeaningEnum cm = _egs.getConstraintMeaning()[basis_ind];
      if (!(cm == GeochemicalSystem::ConstraintMeaningEnum::ACTIVITY ||
            cm == GeochemicalSystem::ConstraintMeaningEnum::FUGACITY))
        paramError("remove_fixed_activity_name",
                   "The species ",
                   _remove_fixed_activity_name[i],
                   " is does not have an activity or fugacity constraint so cannot have such a "
                   "constraint removed");
    }
  }
  if (_num_removed_fixed != _remove_fixed_activity_time.size())
    paramError("remove_fixed_activity_name",
               "must be of the same size as remove_fixed_activity_time");
  if (coupledComponents("controlled_activity_value") != _num_controlled_activity)
    paramError("controlled_activity_name", "must have the same size as controlled_activity_value");

  // record coupled variables
  const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (unsigned int i = 0; i < coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);

  // setup minerals_dumped
  for (unsigned i = 0; i < _mgd.basis_species_name.size(); ++i)
    if (_mgd.basis_species_mineral[i])
      _minerals_dumped[_mgd.basis_species_name[i]] = 0.0;
  for (unsigned j = 0; j < _mgd.eqm_species_name.size(); ++j)
    if (_mgd.eqm_species_mineral[j])
      _minerals_dumped[_mgd.eqm_species_name[j]] = 0.0;
  for (unsigned k = 0; k < _mgd.kin_species_name.size(); ++k)
    if (_mgd.kin_species_mineral[k])
      _minerals_dumped[_mgd.kin_species_name[k]] = 0.0;
}

void
GeochemistryTimeDependentReactor::initialize()
{
  GeochemistryReactorBase::initialize();
}
void
GeochemistryTimeDependentReactor::finalize()
{
  GeochemistryReactorBase::finalize();
}

void
GeochemistryTimeDependentReactor::initialSetup()
{
  // solve the geochemical system with its initial composition and with dt=0 so no kinetic additions
  if (_num_my_nodes == 0)
    return; // rather peculiar case where user has used many processors
  _mole_additions.zero();
  _dmole_additions.zero();
  _solver.solveSystem(_egs,
                      _solver_output[0],
                      _tot_iter[0],
                      _abs_residual[0],
                      0.0,
                      _mole_additions,
                      _dmole_additions);
}

void
GeochemistryTimeDependentReactor::execute()
{
  if (_current_node->id() != 0)
    return;

  _solver.setRampMaxIonicStrength(_ramp_subsequent);

  _mole_additions.zero();
  _dmole_additions.zero();

  // remove appropriate constraints
  if (!_closed_system && _t >= _close_system_at_time)
  {
    _egs.closeSystem();
    _closed_system = true;
  }
  for (unsigned i = 0; i < _num_removed_fixed; ++i)
  {
    if (!_removed_fixed_activity[i] && _t >= _remove_fixed_activity_time[i])
    {
      if (_mgd.basis_species_index.count(_remove_fixed_activity_name[i]))
        _egs.changeConstraintToBulk(_mgd.basis_species_index.at(_remove_fixed_activity_name[i]));
      _removed_fixed_activity[i] = true;
    }
  }

  // control activity
  for (unsigned ca = 0; ca < _num_controlled_activity; ++ca)
  {
    const std::vector<GeochemicalSystem::ConstraintMeaningEnum> & cm = _egs.getConstraintMeaning();
    if (_mgd.basis_species_index.count(_controlled_activity_species_names[ca]))
    {
      const unsigned basis_ind =
          _mgd.basis_species_index.at(_controlled_activity_species_names[ca]);
      if (cm[basis_ind] == GeochemicalSystem::ConstraintMeaningEnum::ACTIVITY ||
          cm[basis_ind] == GeochemicalSystem::ConstraintMeaningEnum::FUGACITY)
        _egs.setConstraintValue(basis_ind, (*_controlled_activity_species_values[ca])[0]);
    }
  }

  // compute moles added
  for (unsigned i = 0; i < _num_source_species; ++i)
  {
    const Real this_rate = (*_source_species_rates[i])[0];
    if (_mgd.basis_species_index.count(_source_species_names[i]))
    {
      const unsigned basis_ind = _mgd.basis_species_index.at(_source_species_names[i]);
      _mole_additions(basis_ind) += this_rate;
    }
    else if (_mgd.eqm_species_index.count(_source_species_names[i]))
    {
      const unsigned eqm_j = _mgd.eqm_species_index.at(_source_species_names[i]);
      for (unsigned basis_ind = 0; basis_ind < _num_basis; ++basis_ind)
        _mole_additions(basis_ind) += _mgd.eqm_stoichiometry(eqm_j, basis_ind) * this_rate;
    }
    else
    {
      const unsigned kin_ind = _mgd.kin_species_index.at(_source_species_names[i]);
      _mole_additions(_num_basis + kin_ind) += this_rate;
    }
  }
  for (unsigned i = 0; i < _num_basis + _num_kin; ++i)
    _mole_additions(i) *= _dt;

  // activate special modes
  if (_mode[0] == 1.0)
    preSolveDump();
  else if (_mode[0] == 3.0)
    preSolveFlush();
  else if (_mode[0] == 4.0)
  {
    removeCurrentSpecies();
    _new_temperature = _cold_temperature[0];
  }
  else // nothing special: simply compute the desired temperature
    _new_temperature = newTemperature(_mole_additions);

  // set temperature, if necessary
  if (_new_temperature != _previous_temperature)
  {
    _egs.setTemperature(_new_temperature);
    _egs.computeConsistentConfiguration();
  }
  _previous_temperature = _new_temperature;

  // solve the geochemical system
  _solver.solveSystem(_egs,
                      _solver_output[0],
                      _tot_iter[0],
                      _abs_residual[0],
                      _dt,
                      _mole_additions,
                      _dmole_additions);

  // activate special modes
  if (_mode[0] == 2.0)
    postSolveFlowThrough();
  else if (_mode[0] == 4.0)
    postSolveExchanger();
}

const GeochemicalSystem &
    GeochemistryTimeDependentReactor::getGeochemicalSystem(dof_id_type /*node_id*/) const
{
  return _egs;
}

const DenseVector<Real> &
    GeochemistryTimeDependentReactor::getMoleAdditions(dof_id_type /*node_id*/) const
{
  return _mole_additions;
}

const std::stringstream &
    GeochemistryTimeDependentReactor::getSolverOutput(dof_id_type /*node_id*/) const
{
  return _solver_output[0];
}

unsigned GeochemistryTimeDependentReactor::getSolverIterations(dof_id_type /*node_id*/) const
{
  return _tot_iter[0];
}

Real GeochemistryTimeDependentReactor::getSolverResidual(dof_id_type /*node_id*/) const
{
  return _abs_residual[0];
}

Real
GeochemistryTimeDependentReactor::getMolesDumped(dof_id_type /*node_id*/,
                                                 const std::string & species) const
{
  if (_minerals_dumped.count(species) == 1)
    return _minerals_dumped.at(species);
  return 0.0;
}

Real
GeochemistryTimeDependentReactor::newTemperature(const DenseVector<Real> & mole_additions) const
{
  if (_temperature[0] == _previous_temperature)
    return _temperature[0];

  // if no reactants are being added, the system temperature will be _temperature[0]
  bool any_additions = false;
  for (unsigned i = 0; i < _num_basis + _num_kin; ++i)
    if (mole_additions(i) > 0)
    {
      any_additions = true;
      break;
    }
  if (!any_additions)
    return _temperature[0];

  // assume heat capacities of inputs and outputs are the same, so final temperature is dictated
  // by masses, also assume that the input happens first, then temperature equilibration, then
  // the outputs occur
  Real new_temperature = _temperature[0];
  const std::vector<Real> & current_bulk = _egs.getBulkMolesOld();
  Real current_kg = current_bulk[0] / GeochemistryConstants::MOLES_PER_KG_WATER;
  Real input_kg = std::max(mole_additions(0), 0.0) / GeochemistryConstants::MOLES_PER_KG_WATER;
  for (unsigned i = 1; i < _num_basis; ++i)
  {
    current_kg += current_bulk[i] * _mgd.basis_species_molecular_weight[i] / 1000.0;
    input_kg += std::max(mole_additions(i), 0.0) * _mgd.basis_species_molecular_weight[i] / 1000.0;
  }
  for (unsigned k = 0; k < _num_kin; ++k)
    input_kg += std::max(mole_additions(k + _num_basis), 0.0) *
                _mgd.kin_species_molecular_weight[k] / 1000.0;
  new_temperature =
      (_previous_temperature * current_kg + _temperature[0] * input_kg) / (current_kg + input_kg);
  return new_temperature;
}

void
GeochemistryTimeDependentReactor::preSolveDump()
{
  // remove basis mineral moles
  const std::vector<Real> & current_molal = _egs.getSolventMassAndFreeMolalityAndMineralMoles();
  for (unsigned i = 1; i < _num_basis; ++i)
    if (_mgd.basis_species_mineral[i])
    {
      _mole_additions(i) = -current_molal[i]; // might overwrite the rates set above, which is good
      _minerals_dumped[_mgd.basis_species_name[i]] += current_molal[i];
    }

  _new_temperature = newTemperature(_mole_additions);

  // add the chemicals immediately instead of during the solve (as occurs for other modes)
  for (unsigned basis_ind = 0; basis_ind < _num_basis; ++basis_ind)
  {
    _egs.addToBulkMoles(basis_ind, _mole_additions(basis_ind));
    _mole_additions(basis_ind) = 0.0;
  }
  // dump needs free mineral moles to be exactly zero and the above addToBulkMoles will have set
  // this for standard minerals, but not things related to sorption or kinetic minerals, so:
  _egs.setMineralRelatedFreeMoles(0.0);

  // Now need to swap all minerals out of the basis
  const std::vector<Real> & eqm_molality = _egs.getEquilibriumMolality();
  unsigned swap_into_basis = 0;
  for (unsigned i = 0; i < _num_basis; ++i)
    if (_mgd.basis_species_mineral[i])
    {
      const bool legitimate_swap_found = _egs.getSwapper().findBestEqmSwap(
          i, _mgd, eqm_molality, false, false, false, swap_into_basis);
      if (legitimate_swap_found)
      {
        try
        {
          _egs.performSwap(i, swap_into_basis);
        }
        catch (const MooseException & e)
        {
          mooseError(e.what());
        }
      }
    }
}

void
GeochemistryTimeDependentReactor::preSolveFlush()
{
  _new_temperature = newTemperature(_mole_additions);

  // Here we conserve mass, so compute the mass of the solution, without the free mineral moles.
  // We don't include the free mineral moles because users of GeochemistWorkbench will want
  // "flush" to operate like Bethke Eqn(13.14)
  // I assume we also don't include kinetic-mineral moles
  Real kg_in = _mole_additions(0) / GeochemistryConstants::MOLES_PER_KG_WATER;
  for (unsigned i = 1; i < _num_basis; ++i)
    if (!_mgd.basis_species_mineral[i])
      kg_in += _mole_additions(i) * _mgd.basis_species_molecular_weight[i] / 1000.0;
  for (unsigned kin = 0; kin < _num_kin; ++kin)
    if (!_mgd.kin_species_mineral[kin])
      kg_in += _mole_additions(kin + _num_basis) * _mgd.kin_species_molecular_weight[kin] / 1000.0;

  const std::vector<Real> & current_bulk = _egs.getBulkMolesOld();
  const std::vector<Real> & current_molal = _egs.getSolventMassAndFreeMolalityAndMineralMoles();
  const std::vector<Real> & kin_moles = _egs.getKineticMoles();

  // compute the current mass, without moles from free minerals and without kinetic minerals
  Real current_kg = current_bulk[0] / GeochemistryConstants::MOLES_PER_KG_WATER;
  for (unsigned i = 1; i < _num_basis; ++i)
  {
    Real kinetic_contribution = 0.0;
    for (unsigned k = 0; k < _num_kin; ++k)
      if (_mgd.kin_species_mineral[k])
        kinetic_contribution += kin_moles[k] * _mgd.kin_stoichiometry(k, i);
    if (_mgd.basis_species_mineral[i])
      current_kg += (current_bulk[i] - current_molal[i] - kinetic_contribution) *
                    _mgd.basis_species_molecular_weight[i] / 1000.0;
    else
      current_kg += (current_bulk[i] - kinetic_contribution) *
                    _mgd.basis_species_molecular_weight[i] / 1000.0;
  }

  const Real fraction_to_remove = kg_in / current_kg;
  for (unsigned i = 0; i < _num_basis; ++i)
  {
    Real all_kinetic_contribution = 0.0;
    for (unsigned k = 0; k < _num_kin; ++k)
      all_kinetic_contribution += kin_moles[k] * _mgd.kin_stoichiometry(k, i);
    if (_mgd.basis_species_mineral[i])
      _mole_additions(i) -=
          fraction_to_remove * (current_bulk[i] - current_molal[i] - all_kinetic_contribution);
    else
      _mole_additions(i) -= fraction_to_remove * (current_bulk[i] - all_kinetic_contribution);
  }
  for (unsigned k = 0; k < _num_kin; ++k)
    if (!_mgd.kin_species_mineral[k])
      _mole_additions(k + _num_basis) -= fraction_to_remove * kin_moles[k];
}

void
GeochemistryTimeDependentReactor::postSolveFlowThrough()
{
  // copy the current_molal values into a new vector
  const std::vector<Real> current_molal = _egs.getSolventMassAndFreeMolalityAndMineralMoles();
  // remove minerals
  for (unsigned i = 1; i < _num_basis; ++i)
    if (_mgd.basis_species_mineral[i])
    {
      const Real to_remove = current_molal[i] - _small_molality * 10.0;
      _egs.addToBulkMoles(i, -to_remove);
      _minerals_dumped[_mgd.basis_species_name[i]] += to_remove;
    }

  // copy the current kinetic moles into a new vector
  const std::vector<Real> kin_moles = _egs.getKineticMoles();
  // remove minerals
  for (unsigned k = 0; k < _num_kin; ++k)
    if (_mgd.kin_species_mineral[k])
    {
      const Real to_remove = kin_moles[k] - _small_molality;
      for (unsigned i = 0; i < _num_basis; ++i)
        if (_mgd.kin_stoichiometry(k, i) != 0)
          _egs.addToBulkMoles(i,
                              -_mgd.kin_stoichiometry(k, i) *
                                  to_remove); // remember bulk moles contains kinetic contributions
      _egs.setKineticMoles(k, _small_molality);
      _minerals_dumped[_mgd.kin_species_name[k]] += to_remove;
    }

  _egs.setMineralRelatedFreeMoles(_small_molality * 10.0);
}

void
GeochemistryTimeDependentReactor::removeCurrentSpecies()
{
  const std::vector<Real> & current_bulk = _egs.getBulkMolesOld();
  for (unsigned i = 0; i < _num_basis; ++i)
    _mole_additions(i) -= current_bulk[i];
  const std::vector<Real> & kin_moles = _egs.getKineticMoles();
  for (unsigned k = 0; k < _num_kin; ++k)
    _mole_additions(k + _num_basis) -= kin_moles[k];
}

void
GeochemistryTimeDependentReactor::postSolveExchanger()
{
  // remove precipitates
  postSolveFlowThrough();

  DenseVector<Real> zero_mole_additions(_num_basis + _num_kin);
  DenseMatrix<Real> zero_dmole_additions(_num_basis + _num_kin, _num_basis + _num_kin);

  const Real del_temperature = (_temperature[0] - _previous_temperature) / _heating_increments;

  for (unsigned incr = 0; incr < _heating_increments; ++incr)
  {
    // set temperature
    _new_temperature = _previous_temperature + del_temperature;
    _egs.setTemperature(_new_temperature);
    _egs.computeConsistentConfiguration();
    _previous_temperature = _new_temperature;

    zero_mole_additions.zero();
    zero_dmole_additions.zero();

    // solve the geochemical system
    _solver.solveSystem(_egs,
                        _solver_output[0],
                        _tot_iter[0],
                        _abs_residual[0],
                        _dt,
                        zero_mole_additions,
                        zero_dmole_additions);

    // remove precipitates
    postSolveFlowThrough();
  }
}
