//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddTimeDependentReactionSolverAction.h"
#include "FEProblem.h"

registerMooseAction("GeochemistryApp", AddTimeDependentReactionSolverAction, "setup_mesh");
registerMooseAction("GeochemistryApp", AddTimeDependentReactionSolverAction, "init_mesh");
registerMooseAction("GeochemistryApp", AddTimeDependentReactionSolverAction, "create_problem");
registerMooseAction("GeochemistryApp", AddTimeDependentReactionSolverAction, "add_output");
registerMooseAction("GeochemistryApp",
                    AddTimeDependentReactionSolverAction,
                    "add_geochemistry_molality_aux");
registerMooseAction("GeochemistryApp",
                    AddTimeDependentReactionSolverAction,
                    "add_geochemistry_reactor");

InputParameters
AddTimeDependentReactionSolverAction::validParams()
{
  InputParameters params = AddGeochemistrySolverAction::validParams();
  params.addParam<unsigned>(
      "ramp_max_ionic_strength_subsequent",
      0,
      "The number of iterations over which to progressively increase the maximum ionic strength "
      "(from zero to max_ionic_strength) during time-stepping.  Unless a great deal occurs in each "
      "time step, this parameter can be set quite small");
  params.addParam<Real>("initial_temperature",
                        25.0,
                        "The initial aqueous solution is equilibrated at this system before adding "
                        "reactants, changing temperature, etc.");
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
  params.addParam<Real>("close_system_at_time",
                        0.0,
                        "Time at which to 'close' the system, that is, change a kg_solvent_water "
                        "constraint to moles_bulk_water, and all free_molality and "
                        "free_moles_mineral_species to moles_bulk_species");
  params.addParam<std::vector<std::string>>(
      "remove_fixed_activity_name",
      "The name of the species that should have their activity or fugacity constraint removed at "
      "time given in remove_fixed_activity_time.  There should be an equal number of these names "
      "as times given in remove_fixed_activity_time");
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
      "an equal number of these names as values given in controlled_activity_value");
  params.addCoupledVar("controlled_activity_value",
                       "Values of the activity or fugacity of the species in "
                       "controlled_activity_name list.  These should always be positive");
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
  params.addClassDescription(
      "Action that sets up a time-dependent equilibrium reaction solver.  This creates creates a "
      "time-dependent geochemistry solver, and adds AuxVariables corresonding to the molalities, "
      "etc");

  return params;
}

AddTimeDependentReactionSolverAction::AddTimeDependentReactionSolverAction(InputParameters params)
  : AddGeochemistrySolverAction(params)
{
}

void
AddTimeDependentReactionSolverAction::act()
{
  // create Output and Aux objects
  AddGeochemistrySolverAction::act();

  // Set up an arbitrary mesh
  if (_current_task == "setup_mesh")
  {
    const std::string class_name = "GeneratedMesh";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<MooseEnum>("dim") = "1";
    _mesh = _factory.create<MooseMesh>(class_name, "mesh", params);
  }
  // Initialize the arbitrary mesh
  else if (_current_task == "init_mesh")
  {
    _mesh->init();
  }
  // Create a "solve=false" FEProblem
  else if (_current_task == "create_problem")
  {
    const std::string class_name = "FEProblem";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<MooseMesh *>("mesh") = _mesh.get();
    params.set<bool>("use_nonlinear") = true;
    params.set<bool>("solve") = false;
    _problem = _factory.create<FEProblemBase>(class_name, "Problem", params);
    _problem->setKernelCoverageCheck(false);
  }
  else if (_current_task == "add_geochemistry_reactor")
  {
    const std::string class_name = "GeochemistryTimeDependentReactor";
    auto params = _factory.getValidParams(class_name);
    // Only pass parameters that were supplied to this action
    params.set<UserObjectName>("model_definition") = getParam<UserObjectName>("model_definition");
    if (isParamValid("swap_out_of_basis"))
      params.set<std::vector<std::string>>("swap_out_of_basis") =
          getParam<std::vector<std::string>>("swap_out_of_basis");
    if (isParamValid("swap_into_basis"))
      params.set<std::vector<std::string>>("swap_into_basis") =
          getParam<std::vector<std::string>>("swap_into_basis");
    params.set<MultiMooseEnum>("constraint_meaning") =
        getParam<MultiMooseEnum>("constraint_meaning");
    params.set<std::vector<std::string>>("constraint_species") =
        getParam<std::vector<std::string>>("constraint_species");
    params.set<std::vector<Real>>("constraint_value") =
        getParam<std::vector<Real>>("constraint_value");
    params.set<Real>("max_ionic_strength") = getParam<Real>("max_ionic_strength");
    params.set<unsigned>("extra_iterations_to_make_consistent") =
        getParam<unsigned>("extra_iterations_to_make_consistent");
    params.applySpecificParameters(parameters(), {"temperature"});
    params.set<Real>("stoichiometry_tolerance") = getParam<Real>("stoichiometry_tolerance");
    params.set<std::string>("charge_balance_species") =
        getParam<std::string>("charge_balance_species");
    if (isParamValid("prevent_precipitation"))
      params.set<std::vector<std::string>>("prevent_precipitation") =
          getParam<std::vector<std::string>>("prevent_precipitation");
    params.set<Real>("abs_tol") = getParam<Real>("abs_tol");
    params.set<Real>("rel_tol") = getParam<Real>("rel_tol");
    params.set<Real>("min_initial_molality") = getParam<Real>("min_initial_molality");
    params.set<unsigned>("max_iter") = getParam<unsigned>("max_iter");
    params.set<Real>("max_initial_residual") = getParam<Real>("max_initial_residual");
    params.set<Real>("swap_threshold") = getParam<Real>("swap_threshold");
    params.set<unsigned>("max_swaps_allowed") = getParam<unsigned>("max_swaps_allowed");
    params.set<unsigned>("ramp_max_ionic_strength_initial") =
        getParam<unsigned>("ramp_max_ionic_strength_initial");
    params.set<unsigned>("ramp_max_ionic_strength_subsequent") =
        getParam<unsigned>("ramp_max_ionic_strength_subsequent");
    params.set<bool>("ionic_str_using_basis_only") = getParam<bool>("ionic_str_using_basis_only");
    params.set<bool>("stoichiometric_ionic_str_using_Cl_only") =
        getParam<bool>("stoichiometric_ionic_str_using_Cl_only");
    params.set<Real>("close_system_at_time") = getParam<Real>("close_system_at_time");
    if (isParamValid("remove_fixed_activity_name"))
      params.set<std::vector<std::string>>("remove_fixed_activity_name") =
          getParam<std::vector<std::string>>("remove_fixed_activity_name");
    if (isParamValid("remove_fixed_activity_time"))
      params.set<std::vector<Real>>("remove_fixed_activity_time") =
          getParam<std::vector<Real>>("remove_fixed_activity_time");
    if (isParamValid("source_species_names"))
      params.set<std::vector<std::string>>("source_species_names") =
          getParam<std::vector<std::string>>("source_species_names");
    if (isParamValid("source_species_rates"))
      params.applySpecificParameters(parameters(), {"source_species_rates"});
    if (isParamValid("controlled_activity_name"))
      params.set<std::vector<std::string>>("controlled_activity_name") =
          getParam<std::vector<std::string>>("controlled_activity_name");
    if (isParamValid("controlled_activity_value"))
      params.applySpecificParameters(parameters(), {"controlled_activity_value"});
    params.applySpecificParameters(parameters(), {"mode"});
    params.set<Real>("initial_temperature") = getParam<Real>("initial_temperature");
    params.set<bool>("evaluate_kinetic_rates_always") =
        getParam<bool>("evaluate_kinetic_rates_always");
    if (isParamValid("kinetic_species_name"))
      params.set<std::vector<std::string>>("kinetic_species_name") =
          getParam<std::vector<std::string>>("kinetic_species_name");
    if (isParamValid("kinetic_species_initial_moles"))
      params.set<std::vector<Real>>("kinetic_species_initial_moles") =
          getParam<std::vector<Real>>("kinetic_species_initial_moles");
    params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END};
    _problem->addUserObject(
        class_name, getParam<UserObjectName>("geochemistry_reactor_name"), params);
  }
}
