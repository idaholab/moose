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
      "This may vary temporally.  If mode=1 then 'dump' mode is used, which means "
      "all mineral masses are removed from the system before the equilibrium solution is sought "
      "(ie, removal occurs at the beginning of the time step).  If mode=2 then 'flow-through' mode "
      "is used, which means all mimeral masses are removed from the system after it the equilbrium "
      "solution has been found (ie, at the end of a time step).  If mode=3 then 'flush' mode is "
      "used, then before the equilibrium solution is sought (ie, at the start of a time step) "
      "water+species is removed from the system at the same rate as pure water + non-mineral "
      "solutes are entering the system (specified in source_species_rates).  If mode is any other "
      "number, no special mode is active (the system simply responds to the source_species_rates, "
      "controlled_activity_value, etc).");
  params.addCoupledVar(
      "temperature",
      25,
      "Temperature.  Note, this has two different meanings.  (1) If no species are being added to "
      "the solution (no source_species_rates are positive) then this is the temperature of the "
      "aqueous solution.  (2) If species are being added, this is the temperature of the species "
      "being added.  In case (2), the final aqueous-solution temperature is computed assuming the "
      "species are added, temperature is equilibrated and then, if species are also being removed, "
      "they are removed.  If you wish to add species and simultaneously alter the temperature, you "
      "will have to use a sequence of heat-add-heat-add, etc steps");
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
      "Names of the kinetic species given initial values in kinetic_species_initial_moles");
  params.addParam<std::vector<Real>>(
      "kinetic_species_initial_moles",
      "Initial number of moles for each of the species named in kinetic_species_name");
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
         getParam<MultiMooseEnum>("constraint_meaning"),
         _previous_temperature,
         getParam<unsigned>("extra_iterations_to_make_consistent"),
         getParam<Real>("min_initial_molality"),
         getParam<std::vector<std::string>>("kinetic_species_name"),
         getParam<std::vector<Real>>("kinetic_species_initial_moles")),
    _solver(_mgd,
            _egs,
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
    _source_species_rates(0),
    _remove_fixed_activity_name(getParam<std::vector<std::string>>("remove_fixed_activity_name")),
    _remove_fixed_activity_time(getParam<std::vector<Real>>("remove_fixed_activity_time")),
    _num_removed_fixed(_remove_fixed_activity_name.size()),
    _removed_fixed_activity(_num_removed_fixed, false),
    _controlled_activity_species_names(
        getParam<std::vector<std::string>>("controlled_activity_name")),
    _num_controlled_activity(_controlled_activity_species_names.size()),
    _controlled_activity_species_values(0),
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
    _source_species_rates.push_back(&coupledValue("source_species_rates", i));
  for (unsigned i = 0; i < _num_source_species; ++i)
    if (!(_mgd.basis_species_index.count(_source_species_names[i]) == 1 ||
          _mgd.eqm_species_index.count(_source_species_names[i]) == 1))
      paramError("source_species_names",
                 "The name " + _source_species_names[i] +
                     " does not appear in the basis or equilibrium species list");

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
  for (unsigned i = 0; i < _num_controlled_activity; ++i)
    _controlled_activity_species_values.push_back(&coupledValue("controlled_activity_value", i));

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
  _mole_additions.zero();
  _dmole_additions.zero();
  _solver.solveSystem(
      _solver_output, _tot_iter, _abs_residual, 0.0, _mole_additions, _dmole_additions);
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
    else
    {
      const unsigned eqm_j = _mgd.eqm_species_index.at(_source_species_names[i]);
      for (unsigned basis_ind = 0; basis_ind < _num_basis; ++basis_ind)
        _mole_additions(basis_ind) += _mgd.eqm_stoichiometry(eqm_j, basis_ind) * this_rate;
    }
  }
  for (unsigned basis_ind = 0; basis_ind < _num_basis; ++basis_ind)
    _mole_additions(basis_ind) *= _dt;

  // activate special modes
  if (_mode[0] == 1.0) // dump
  {
    const std::vector<Real> & current_molal = _egs.getSolventMassAndFreeMolalityAndMineralMoles();
    for (unsigned i = 1; i < _num_basis; ++i)
      if (_mgd.basis_species_mineral[i])
      {
        _mole_additions(i) =
            -current_molal[i]; // might overwrite the rates set above, which is good
        _minerals_dumped[_mgd.basis_species_name[i]] += current_molal[i];
      }
  }
  else if (_mode[0] == 3.0) // flush
  {
    // Here we conserve mass, so compute the mass of the solution, without the free mineral moles.
    // We don't include the free mineral moles because users of GeochemistWorkbench will want
    // "flush" to operate like Bethke Eqn(13.14)
    // I assume we also don't include kinetic-mineral moles
    Real kg_in = _mole_additions(0) / GeochemistryConstants::MOLES_PER_KG_WATER;
    for (unsigned i = 1; i < _num_basis; ++i)
      if (!_mgd.basis_species_mineral[i])
        kg_in += _mole_additions(i) * _mgd.basis_species_molecular_weight[i] / 1000.0;

    const std::vector<Real> & current_bulk = _egs.getBulkMolesOld();
    const std::vector<Real> & current_molal = _egs.getSolventMassAndFreeMolalityAndMineralMoles();
    const std::vector<Real> & kin_moles = _egs.getKineticMoles();

    // compute the current mass, without moles from free minerals and without kinetic minerals
    Real current_kg = current_bulk[0] / GeochemistryConstants::MOLES_PER_KG_WATER;
    for (unsigned i = 1; i < _num_basis; ++i)
    {
      Real kinetic_contribution = 0.0;
      for (unsigned k = 0; k < _num_kin; ++k)
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
      Real kinetic_contribution = 0.0;
      for (unsigned k = 0; k < _num_kin; ++k)
        kinetic_contribution += kin_moles[k] * _mgd.kin_stoichiometry(k, i);
      if (_mgd.basis_species_mineral[i])
        _mole_additions(i) -=
            fraction_to_remove * (current_bulk[i] - current_molal[i] - kinetic_contribution);
      else
        _mole_additions(i) -= fraction_to_remove * (current_bulk[i] - kinetic_contribution);
    }
  }

  Real new_temperature = _temperature[0];
  if (new_temperature != _previous_temperature)
  {
    // if reactants are being added, the system temperature will not be _temperature[0]
    bool any_additions = false;
    for (unsigned basis_ind = 0; basis_ind < _num_basis; ++basis_ind)
      if (_mole_additions(basis_ind) > 0)
      {
        any_additions = true;
        break;
      }
    if (any_additions)
    {
      // assume heat capacities of inputs and outputs are the same, so final temperature is dictated
      // by masses, also assume that the input happens first, then temperature equilibration, then
      // the outputs occur
      const std::vector<Real> & current_bulk = _egs.getBulkMolesOld();
      Real current_kg = current_bulk[0] / GeochemistryConstants::MOLES_PER_KG_WATER;
      Real input_kg = std::max(_mole_additions(0), 0.0) / GeochemistryConstants::MOLES_PER_KG_WATER;
      for (unsigned i = 1; i < _num_basis; ++i)
      {
        current_kg += current_bulk[i] * _mgd.basis_species_molecular_weight[i] / 1000.0;
        input_kg +=
            std::max(_mole_additions(i), 0.0) * _mgd.basis_species_molecular_weight[i] / 1000.0;
      }
      new_temperature = (_previous_temperature * current_kg + _temperature[0] * input_kg) /
                        (current_kg + input_kg);
    }
  }

  if (_mode[0] == 1.0) // dump all minerals
  {
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

  // set temperature, if necessary
  if (new_temperature != _previous_temperature)
  {
    _egs.setTemperature(new_temperature);
    _egs.computeConsistentConfiguration();
  }
  _previous_temperature = new_temperature;

  // solve the geochemical system
  _solver.solveSystem(
      _solver_output, _tot_iter, _abs_residual, _dt, _mole_additions, _dmole_additions);

  if (_mode[0] == 2.0) // flow-through
  {
    // copy the current_molal values into a new vector
    const std::vector<Real> current_molal = _egs.getSolventMassAndFreeMolalityAndMineralMoles();
    for (unsigned i = 1; i < _num_basis; ++i)
      if (_mgd.basis_species_mineral[i])
      {
        _egs.addToBulkMoles(i, -current_molal[i]);
        _minerals_dumped[_mgd.basis_species_name[i]] += current_molal[i];
      }
    _egs.setMineralRelatedFreeMoles(_small_molality * 10.0);
  }
}

const GeochemicalSystem &
GeochemistryTimeDependentReactor::getGeochemicalSystem(const Point & /*point*/) const
{
  return _egs;
}

const GeochemicalSystem &
GeochemistryTimeDependentReactor::getGeochemicalSystem(unsigned /*node_id*/) const
{
  return _egs;
}

const DenseVector<Real> &
GeochemistryTimeDependentReactor::getMoleAdditions(unsigned /*node_id*/) const
{
  return _mole_additions;
}

const DenseVector<Real> &
GeochemistryTimeDependentReactor::getMoleAdditions(const Point & /*point*/) const
{
  return _mole_additions;
}

const std::stringstream &
GeochemistryTimeDependentReactor::getSolverOutput(const Point & /*point*/) const
{
  return _solver_output;
}

unsigned
GeochemistryTimeDependentReactor::getSolverIterations(const Point & /*point*/) const
{
  return _tot_iter;
}

Real
GeochemistryTimeDependentReactor::getSolverResidual(const Point & /*point*/) const
{
  return _abs_residual;
}

Real
GeochemistryTimeDependentReactor::getMolesDumped(unsigned /*node_id*/,
                                                 const std::string & species) const
{
  if (_minerals_dumped.count(species) == 1)
    return _minerals_dumped.at(species);
  return 0.0;
}
