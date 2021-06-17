//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistrySpatialReactor.h"

registerMooseObject("GeochemistryApp", GeochemistrySpatialReactor);

InputParameters
GeochemistrySpatialReactor::sharedParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<unsigned>(
      "ramp_max_ionic_strength_subsequent",
      0,
      "The number of iterations over which to progressively increase the maximum ionic strength "
      "(from zero to max_ionic_strength) during time-stepping.  Unless a great deal occurs in each "
      "time step, this parameter can be set quite small");
  params.addParam<Real>("initial_temperature",
                        25,
                        "Temperature at which the initial system is equilibrated.  This is uniform "
                        "over the entire mesh.");
  params.addCoupledVar("temperature", 25, "Temperature");
  params.addParam<Real>("close_system_at_time",
                        0.0,
                        "Time at which to 'close' the entire spatial system, that is, change a "
                        "kg_solvent_water constraint to moles_bulk_water, and all free_molality "
                        "and free_moles_mineral_species to moles_bulk_species");
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
  params.addParam<bool>("adaptive_timestepping",
                        false,
                        "Use adaptive timestepping at each node in an attempt to ensure "
                        "convergence of the solver.  Setting this parameter to false saves some "
                        "compute time because copying of datastructures is avoided");
  params.addParam<Real>(
      "dt_min",
      1E-10,
      "If, during adaptive timestepping at a node, the time-step fails below this value, "
      "MOOSE will give up trying to solve the geochemical system.  Optimally, you should set this "
      "value bearing abs_tol in mind because as dt changes, the initial value of the residual will "
      "also typically change.  For example, if you set dt_min very small relative to abs_tol MOOSE "
      "may think the system has converged just because dt is small.  Also, bear in mind your "
      "typical timestep size: if dt_min < 1E-16*typical_dt then you will run out of precision");
  params.addRangeCheckedParam<Real>(
      "dt_dec",
      0.5,
      "dt_dec >= 0 & dt_dec < 1.0",
      "If a geochemical solve fails, then 'adpative timestepping' at the node is initiated "
      "(assuming adaptive_timestepping = true): the time-step at the node is multiplied by this "
      "amount, and the solve process re-tried");
  params.addRangeCheckedParam<Real>(
      "dt_inc",
      1.1,
      "dt_inc >= 1.0",
      "If a geochemical solve suceeds during adpative timestepping at a node, then the time-step "
      "at the node is multiplied by this amount before performing the next adaptive timestep");
  return params;
}

InputParameters
GeochemistrySpatialReactor::validParams()
{
  InputParameters params = GeochemistryReactorBase::validParams();
  params += GeochemistrySpatialReactor::sharedParams();
  params.addClassDescription("UserObject that controls the space-dependent and time-dependent "
                             "geochemistry reaction processes");
  return params;
}

GeochemistrySpatialReactor::GeochemistrySpatialReactor(const InputParameters & parameters)
  : GeochemistryReactorBase(parameters),
    _initial_temperature(getParam<Real>("initial_temperature")),
    _temperature(coupledValue("temperature")),
    _num_kin(_mgd.kin_species_name.size()),
    // NOTE: initialize _mgd_at_node before the swaps are performed
    _mgd_at_node(_num_my_nodes, _mgd),
    _egs_at_node(),
    // NOTE: the following implements the swaps in _mgd
    _egs_copy(_mgd,
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
              _initial_temperature,
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
    _close_system_at_time(getParam<Real>("close_system_at_time")),
    _closed_system(false),
    _source_species_names(getParam<std::vector<std::string>>("source_species_names")),
    _num_source_species(_source_species_names.size()),
    _source_species_rates(0),
    _remove_fixed_activity_name(getParam<std::vector<std::string>>("remove_fixed_activity_name")),
    _remove_fixed_activity_time(getParam<std::vector<Real>>("remove_fixed_activity_time")),
    _num_removed_fixed(_remove_fixed_activity_name.size()),
    _removed_fixed_activity(_num_my_nodes, std::vector<bool>(_num_removed_fixed, false)),
    _controlled_activity_species_names(
        getParam<std::vector<std::string>>("controlled_activity_name")),
    _num_controlled_activity(_controlled_activity_species_names.size()),
    _controlled_activity_species_values(0),
    _mole_rates(_num_basis + _num_kin),
    _mole_additions(_num_my_nodes, DenseVector<Real>(_num_basis + _num_kin)),
    _dmole_additions(_num_my_nodes,
                     DenseMatrix<Real>(_num_basis + _num_kin, _num_basis + _num_kin)),
    _ramp_subsequent(getParam<unsigned>("ramp_max_ionic_strength_subsequent")),
    _my_node_number(),
    _execute_done(_num_my_nodes, false),
    _adaptive_timestepping(getParam<bool>("adaptive_timestepping")),
    _dt_min(_adaptive_timestepping ? getParam<Real>("dt_min") : std::numeric_limits<Real>::max()),
    _dt_dec(getParam<Real>("dt_dec")),
    _dt_inc(getParam<Real>("dt_inc")),
    _nthreads(1)
{
  // build _egs_at_node
  for (unsigned i = 0; i < _num_my_nodes; ++i)
    _egs_at_node.push_back(
        GeochemicalSystem(_mgd_at_node[i],
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
                          _initial_temperature,
                          getParam<unsigned>("extra_iterations_to_make_consistent"),
                          getParam<Real>("min_initial_molality"),
                          getParam<std::vector<std::string>>("kinetic_species_name"),
                          getParam<std::vector<Real>>("kinetic_species_initial_value"),
                          getParam<MultiMooseEnum>("kinetic_species_unit")));

  // check sources and set the rates
  if (coupledComponents("source_species_rates") != _num_source_species)
    paramError("source_species_names", "must have the same size as source_species_rates");
  for (unsigned i = 0; i < _num_source_species; ++i)
    _source_species_rates.push_back(&coupledValue("source_species_rates", i));
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
    else if (_num_my_nodes >
             0) // don't consider the silly (but possible) case that there are no nodes
    {
      const unsigned basis_ind = _mgd.basis_species_index.at(_remove_fixed_activity_name[i]);
      const GeochemicalSystem::ConstraintMeaningEnum cm =
          _egs_at_node[0].getConstraintMeaning()[basis_ind];
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

  buildMyNodeNumber();
}

void
GeochemistrySpatialReactor::buildMyNodeNumber()
{
  const MeshBase & msh = _subproblem.mesh().getMesh();

  // create the map from MOOSE's node numbering to the local node numbering (_my_node_number) used
  // by this object
  _my_node_number.clear();
  unsigned num_nodes_inserted = 0;
  for (const auto & node : as_range(msh.local_nodes_begin(), msh.local_nodes_end()))
  {
    if (_my_node_number.count(node->id()) == 0)
      _my_node_number[node->id()] = num_nodes_inserted;
    else
      mooseError(
          "GeochemistrySpatialReactor: something wrong with node numbering in buildMyNodeNumber");
    num_nodes_inserted += 1;
  }
  mooseAssert(
      _my_node_number.size() == _num_my_nodes,
      "GeochemistrySpatialReactor: something wrong with node numbering in buildMyNodeNumber");
}

void
GeochemistrySpatialReactor::initialSetup()
{
  // Solve the geochemical system with its initial composition and with dt=0 so no kinetic additions
  for (unsigned i = 0; i < _num_my_nodes; ++i)
  {
    _mole_additions[i].zero();
    _dmole_additions[i].zero();
    _solver.solveSystem(_egs_at_node[i],
                        _solver_output[i],
                        _tot_iter[i],
                        _abs_residual[i],
                        0.0,
                        _mole_additions[i],
                        _dmole_additions[i]);
  }
}

void
GeochemistrySpatialReactor::initialize()
{
  GeochemistryReactorBase::initialize();
  _execute_done.assign(_num_my_nodes, false);
  _nthreads = 1;
}

void
GeochemistrySpatialReactor::execute()
{
  if (_my_node_number.count(_current_node->id()) == 0)
    mooseError(
        "GeochemistrySpatialReactor: something wrong with node numbering in buildMyNodeNumber");
  const unsigned my_node_number = _my_node_number.at(_current_node->id());

  const unsigned aux_comp_number = 0; // component number to use for AuxVariables
  const ModelGeochemicalDatabase & mgd_ref =
      _egs_at_node[my_node_number].getModelGeochemicalDatabase();

  // close system
  if (!_closed_system && _t >= _close_system_at_time)
    _egs_at_node[my_node_number].closeSystem();

  // remove fixed-activity constraints.
  for (unsigned i = 0; i < _num_removed_fixed; ++i)
  {
    if (!_removed_fixed_activity[my_node_number][i] && _t >= _remove_fixed_activity_time[i])
    {
      if (mgd_ref.basis_species_index.count(_remove_fixed_activity_name[i]))
        _egs_at_node[my_node_number].changeConstraintToBulk(
            mgd_ref.basis_species_index.at(_remove_fixed_activity_name[i]));
      _removed_fixed_activity[my_node_number][i] = true;
    }
  }

  // control activity
  for (unsigned ca = 0; ca < _num_controlled_activity; ++ca)
  {
    const std::vector<GeochemicalSystem::ConstraintMeaningEnum> & cm =
        _egs_at_node[my_node_number].getConstraintMeaning();
    if (mgd_ref.basis_species_index.count(_controlled_activity_species_names[ca]))
    {
      const unsigned basis_ind =
          mgd_ref.basis_species_index.at(_controlled_activity_species_names[ca]);
      if (cm[basis_ind] == GeochemicalSystem::ConstraintMeaningEnum::ACTIVITY ||
          cm[basis_ind] == GeochemicalSystem::ConstraintMeaningEnum::FUGACITY)
        _egs_at_node[my_node_number].setConstraintValue(
            basis_ind, (*_controlled_activity_species_values[ca])[aux_comp_number]);
    }
  }

  _solver.setRampMaxIonicStrength(_ramp_subsequent);

  Real temperature0 = _egs_at_node[my_node_number].getTemperature();
  const Real temperature_rate = (_temperature[aux_comp_number] - temperature0) / _dt;

  // record the system in case of solve failures using the copy-assignment operator of
  // GeochemicalSystem
  if (_adaptive_timestepping)
    _egs_copy = _egs_at_node[my_node_number];

  Real done_dt = 0.0;
  Real my_dt = _dt;

  // the following loop implements adaptive timestepping at the node
  while (done_dt < _dt)
  {
    // compute moles added in the current basis (the basis might change during adaptive
    // timestepping)
    _mole_rates.zero();
    for (unsigned i = 0; i < _num_source_species; ++i)
    {
      const Real this_rate = (*_source_species_rates[i])[aux_comp_number];
      if (mgd_ref.basis_species_index.count(_source_species_names[i]))
      {
        const unsigned basis_ind = mgd_ref.basis_species_index.at(_source_species_names[i]);
        _mole_rates(basis_ind) += this_rate;
      }
      else if (mgd_ref.eqm_species_index.count(_source_species_names[i]))
      {
        const unsigned eqm_j = mgd_ref.eqm_species_index.at(_source_species_names[i]);
        for (unsigned basis_ind = 0; basis_ind < _num_basis; ++basis_ind)
          _mole_rates(basis_ind) += mgd_ref.eqm_stoichiometry(eqm_j, basis_ind) * this_rate;
      }
      else
      {
        const unsigned kin_ind = mgd_ref.kin_species_index.at(_source_species_names[i]);
        _mole_rates(_num_basis + kin_ind) += this_rate;
      }
    }

    Real temperature = temperature0 + my_dt * temperature_rate;
    for (unsigned i = 0; i < _num_basis + _num_kin; ++i)
      _mole_additions[my_node_number](i) = my_dt * _mole_rates(i);
    _dmole_additions[my_node_number].zero();

    // set temperature, if needed
    if (temperature != _egs_at_node[my_node_number].getTemperature())
    {
      _egs_at_node[my_node_number].setTemperature(temperature);
      _egs_at_node[my_node_number].computeConsistentConfiguration();
    }

    // solve the geochemical system
    try
    {
      _solver.solveSystem(_egs_at_node[my_node_number],
                          _solver_output[my_node_number],
                          _tot_iter[my_node_number],
                          _abs_residual[my_node_number],
                          my_dt,
                          _mole_additions[my_node_number],
                          _dmole_additions[my_node_number]);
      done_dt += my_dt;
      if (done_dt < _dt)
      {
        temperature0 = _egs_at_node[my_node_number].getTemperature();
        _egs_copy =
            _egs_at_node[my_node_number]; // use the copy-assignment operator of GeochemicalSystem
      }
      my_dt *= _dt_inc;
    }
    catch (const MooseException & e)
    {
      // use the copy-assignment operator of GeochemicalSystem to restore to the original
      if (_adaptive_timestepping)
        _egs_at_node[my_node_number] = _egs_copy;
      my_dt *= _dt_dec;
      if (my_dt < _dt_min)
        mooseException(
            "Geochemistry solve failed with dt = ", my_dt, " at node: ", _current_node->get_info());
    }

    if (done_dt + my_dt > _dt)
      my_dt = _dt - done_dt; // avoid overstepping
  }

  _execute_done[my_node_number] = true;
}

void
GeochemistrySpatialReactor::threadJoin(const UserObject & uo)
{
  _nthreads += 1;
  const GeochemistrySpatialReactor & gsr = static_cast<const GeochemistrySpatialReactor &>(uo);
  for (unsigned i = 0; i < _num_my_nodes; ++i)
  {
    if (!_execute_done[i] && gsr._execute_done[i])
    {
      _solver_output[i].str("");
      _solver_output[i] << gsr._solver_output[i].str();
      _tot_iter[i] = gsr._tot_iter[i];
      _abs_residual[i] = gsr._abs_residual[i];
      _mole_additions[i] = gsr._mole_additions[i];
      _egs_at_node[i] = gsr._egs_at_node[i];
      _removed_fixed_activity[i] = gsr._removed_fixed_activity[i];
      // _mgd_at_node does not need to be threadJoined, because _egs_at_node[i] =
      // gsr._egs_at_node[i] uses the copy-assignment operator to copy the data in
      // _egs_at_node[i]._mgd
    }
  }
}

void
GeochemistrySpatialReactor::finalize()
{
  GeochemistryReactorBase::finalize();
  // if relevant, record that system is closed
  if (!_closed_system && _t >= _close_system_at_time)
    _closed_system = true;
  // ensure that the non-main threads have the main-thread's copy of _egs_at_node (and hence
  // _mgd_at_node) and _removed_fixed_activity, since the main-thread's copy has correctly gathered
  // all the information during threadJoin
  for (unsigned thrd = 1; thrd < _nthreads; ++thrd)
  {
    std::vector<GeochemistrySpatialReactor *> objects;
    _fe_problem.theWarehouse()
        .query()
        .condition<AttribSystem>("UserObject")
        .condition<AttribThread>(thrd)
        .condition<AttribName>(name())
        .queryInto(objects);
    mooseAssert(objects.size() == 1,
                "GeochemistrySpatialReactor::finalize() failed to obtain a single thread copy of "
                "the GeochemistrySpatialReactor");
    objects[0]->_removed_fixed_activity = _removed_fixed_activity;
    objects[0]->_egs_at_node = _egs_at_node;
    objects[0]->_closed_system = _closed_system;
  }
}

void
GeochemistrySpatialReactor::meshChanged()
{
  mooseError("GeochemistrySpatialReactor cannot yet handle adaptive meshing");
  /*
Note to future coders:
- have to rebuild _my_node_number.  _num_my_nodes has to be changed (so its not const anymore).
The Action must not just execute_on = EXEC_INITIAL for NearestNodeNumberUO
- have to populate the new nodes correctly.  This might be easiest if, at the start of execute,
_egs_at_node was always populated using AuxVariables (probably a RequiredCoupledVar that is
actually a ArrayVariableValue & (constructed with coupledArrayValue instead of just coupledValue)
that record the kg_solvent_water, free molality, surface_pot_expr, etc.  That is use
_egs_at_node[i].setSolventMassAndFreeMolalityAndMineralMolesAndSurfacePotsAndKineticMoles with the
values from the AuxVariables.  The reason for this design is that then MOOSE handles the
interpolation of the AuxVariables to the newly-created nodes during mesh adaptivity.  The
difficult thing is to figure out the variables at the new nodes.  Probably should copy the basis
species, swap stuff, etc (_mgd_at_node) from the nearest original node to the newly-created node.
And then solve the new system to allow basis swaps if appropriate.  This is a bit tricky, hence it
hasn't yet been implemented.
  */
}

const GeochemicalSystem &
GeochemistrySpatialReactor::getGeochemicalSystem(dof_id_type node_id) const
{
  if (_my_node_number.count(node_id) != 1)
    mooseError("GeochemistrySpatialReactor does not know about node ", node_id);
  return _egs_at_node[_my_node_number.at(node_id)];
}

const DenseVector<Real> &
GeochemistrySpatialReactor::getMoleAdditions(dof_id_type node_id) const
{
  if (_my_node_number.count(node_id) != 1)
    mooseError("GeochemistrySpatialReactor does not know about node ", node_id);
  return _mole_additions[_my_node_number.at(node_id)];
}

const std::stringstream &
GeochemistrySpatialReactor::getSolverOutput(dof_id_type node_id) const
{
  if (_my_node_number.count(node_id) != 1)
    mooseError("GeochemistrySpatialReactor does not know about node ", node_id);
  return _solver_output[_my_node_number.at(node_id)];
}

unsigned
GeochemistrySpatialReactor::getSolverIterations(dof_id_type node_id) const
{
  if (_my_node_number.count(node_id) != 1)
    mooseError("GeochemistrySpatialReactor does not know about node ", node_id);
  return _tot_iter[_my_node_number.at(node_id)];
}

Real
GeochemistrySpatialReactor::getSolverResidual(dof_id_type node_id) const
{
  if (_my_node_number.count(node_id) != 1)
    mooseError("GeochemistrySpatialReactor does not know about node ", node_id);
  return _abs_residual[_my_node_number.at(node_id)];
}

Real
GeochemistrySpatialReactor::getMolesDumped(dof_id_type /*node_id*/,
                                           const std::string & /*species*/) const
{
  return 0.0;
}
