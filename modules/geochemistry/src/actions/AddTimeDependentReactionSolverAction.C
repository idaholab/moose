//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddTimeDependentReactionSolverAction.h"
#include "GeochemistryTimeDependentReactor.h"
#include "FEProblem.h"

registerMooseAction("GeochemistryApp", AddTimeDependentReactionSolverAction, "setup_mesh");
registerMooseAction("GeochemistryApp", AddTimeDependentReactionSolverAction, "init_mesh");
registerMooseAction("GeochemistryApp", AddTimeDependentReactionSolverAction, "create_problem");
registerMooseAction("GeochemistryApp", AddTimeDependentReactionSolverAction, "add_output");
registerMooseAction("GeochemistryApp", AddTimeDependentReactionSolverAction, "add_user_object");
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
  params += GeochemistryTimeDependentReactor::sharedParams();
  params.addClassDescription(
      "Action that sets up a time-dependent equilibrium reaction solver.  This creates creates a "
      "time-dependent geochemistry solver, and adds AuxVariables corresonding to the molalities, "
      "etc");

  return params;
}

AddTimeDependentReactionSolverAction::AddTimeDependentReactionSolverAction(
    const InputParameters & params)
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
  // Create a "solve=false" FEProblem, if appropriate
  else if (_current_task == "create_problem")
  {
    const std::string class_name = "FEProblem";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<MooseMesh *>("mesh") = _mesh.get();
    params.set<bool>("use_nonlinear") = true;
    params.set<bool>("solve") = getParam<bool>("include_moose_solve");
    _problem = _factory.create<FEProblemBase>(class_name, "Problem", params);
    _problem->setKernelCoverageCheck(getParam<bool>("include_moose_solve")
                                         ? FEProblemBase::CoverageCheckMode::TRUE
                                         : FEProblemBase::CoverageCheckMode::FALSE);
  }
  else if (_current_task == "add_geochemistry_reactor")
  {
    const std::string class_name = "GeochemistryTimeDependentReactor";
    auto params = _factory.getValidParams(class_name);
    // Only pass parameters that were supplied to this action
    if (isParamValid("block"))
      params.set<std::vector<SubdomainName>>("block") =
          getParam<std::vector<SubdomainName>>("block");
    if (isParamValid("boundary"))
      params.set<std::vector<BoundaryName>>("boundary") =
          getParam<std::vector<BoundaryName>>("boundary");
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
    params.set<MultiMooseEnum>("constraint_unit") = getParam<MultiMooseEnum>("constraint_unit");
    params.set<Real>("max_ionic_strength") = getParam<Real>("max_ionic_strength");
    params.set<unsigned>("extra_iterations_to_make_consistent") =
        getParam<unsigned>("extra_iterations_to_make_consistent");
    params.applySpecificParameters(parameters(), {"temperature"});
    params.applySpecificParameters(parameters(), {"cold_temperature"});
    params.set<unsigned>("heating_increments") = getParam<unsigned>("heating_increments");
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
    if (isParamValid("kinetic_species_initial_value"))
      params.set<std::vector<Real>>("kinetic_species_initial_value") =
          getParam<std::vector<Real>>("kinetic_species_initial_value");
    if (isParamValid("kinetic_species_unit"))
      params.set<MultiMooseEnum>("kinetic_species_unit") =
          getParam<MultiMooseEnum>("kinetic_species_unit");
    params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END};
    _problem->addUserObject(
        class_name, getParam<UserObjectName>("geochemistry_reactor_name"), params);
  }
}
