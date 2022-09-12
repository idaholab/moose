//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddGeochemistrySolverAction.h"
#include "GeochemicalModelDefinition.h"
#include "GeochemistryReactorBase.h"
#include "NearestNodeNumber.h"
#include "GeochemistryConsoleOutput.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"

registerMooseAction("GeochemistryApp", AddGeochemistrySolverAction, "add_output");
registerMooseAction("GeochemistryApp", AddGeochemistrySolverAction, "add_user_object");
registerMooseAction("GeochemistryApp",
                    AddGeochemistrySolverAction,
                    "add_geochemistry_molality_aux");

InputParameters
AddGeochemistrySolverAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<UserObjectName>(
      "geochemistry_reactor_name",
      "geochemistry_reactor",
      "The name that will be given to the GeochemistryReactor UserObject built by this action");
  params.addParam<bool>("include_moose_solve",
                        false,
                        "Include a usual MOOSE solve involving Variables and Kernels.  In pure "
                        "reaction systems (without transport) include_moose_solve = false is "
                        "appropriate, but with transport 'true' must be used");

  params.addRequiredParam<UserObjectName>("model_definition",
                                          "The name of the GeochemicalModelDefinition user object "
                                          "(you must create this UserObject yourself)");
  params += GeochemistryReactorBase::sharedParams();

  params += BlockRestrictable::validParams();
  params += BoundaryRestrictable::validParams();

  params.addRangeCheckedParam<Real>(
      "stoichiometry_tolerance",
      1E-6,
      "stoichiometry_tolerance >= 0.0",
      "Swapping involves inverting matrices via a singular value decomposition. During this "
      "process: (1) if abs(singular value) < stoi_tol * L1norm(singular values), then the "
      "matrix is deemed singular (so the basis swap is deemed invalid); (2) if abs(any "
      "stoichiometric coefficient) < stoi_tol then it is set to zero.");

  // following are exclusively for the GeochemistryConsoleOutput
  params += GeochemistryConsoleOutput::sharedParams();
  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum = {EXEC_INITIAL, EXEC_FINAL};
  params.addParam<ExecFlagEnum>(
      "execute_console_output_on", exec_enum, "When to execute the geochemistry console output");
  params.addParam<Point>("point",
                         Point(0.0, 0.0, 0.0),
                         "The geochemistry console output will be regarding the aqueous "
                         "solution at node that is closest to this point");

  // following are the Aux possibilities
  params.addParam<bool>(
      "add_aux_solvent_kg",
      true,
      "Add AuxVariable, called kg_solvent_H2O, that records the kg of solvent water");
  params.addParam<bool>(
      "add_aux_pH", true, "Add AuxVariable, called pH, that records the pH of solvent water");
  params.addParam<bool>(
      "add_aux_molal",
      true,
      "Add AuxVariables measured in molal units (ie mol(species)/kg(solvent_water)).  These are "
      "named molal_name, where 'name' is the species name.  AuxVariables are added for all species "
      "except minerals");
  params.addParam<bool>("add_aux_mg_per_kg",
                        true,
                        "Add AuxVariables measured in mg(species)/kg(solvent_water).  These are "
                        "named mg_per_kg_name, where 'name' is the species name.  AuxVariables are "
                        "added for all species except minerals");
  params.addParam<bool>("add_aux_free_mg",
                        true,
                        "Add AuxVariables for all minerals measured in free mg.  These are named "
                        "free_mg_name, where 'name' is the species name");
  params.addParam<bool>("add_aux_free_cm3",
                        true,
                        "Add AuxVariables for all minerals measured in free cm^3.  These are named "
                        "free_cm3_name, where 'name' is the species name");
  params.addParam<bool>(
      "add_aux_activity",
      true,
      "Add AuxVariables that record the activity for all species (for gas species this equals the "
      "gas fugacity).  These are called activity_name where 'name' is the species name.");
  params.addParam<bool>(
      "add_aux_bulk_moles",
      true,
      "Add AuxVariables that record the number of bulk-composition moles for all species.  Note "
      "that these will be zero for any species not currently in the basis.  These are called "
      "bulk_moles_name where 'name' is the species name.");
  params.addParam<bool>("add_aux_surface_charge",
                        true,
                        "Add AuxVariables, measured in C/m^2, corresponding to specific surface "
                        "charge for each mineral involved in surface sorption.  These are "
                        "surface_charge_name, where 'name' is the mineral name");
  params.addParam<bool>("add_aux_surface_potential",
                        true,
                        "Add AuxVariables, measured in V, corresponding to surface potential "
                        "for each mineral involved in surface sorption.  These are "
                        "surface_potential_name, where 'name' is the mineral name");
  params.addParam<bool>("add_aux_temperature",
                        true,
                        "Add AuxVariable, called solution_temperature, that records the "
                        "temperature of the aqueous solution in degC");
  params.addParam<bool>(
      "add_aux_kinetic_moles",
      true,
      "Add AuxVariables that record the number of moles for all kinetic species.  These are called "
      "moles_name where 'name' is the species name.");
  params.addParam<bool>("add_aux_kinetic_additions",
                        true,
                        "Add AuxVariables that record the rate-of-change (-reaction_rate * dt) for "
                        "all kinetic species.  These are called "
                        "mol_change_name where 'name' is the species name.");
  params.addClassDescription("Base class for an Action that sets up a reaction solver.  This class "
                             "adds a GeochemistryConsoleOutput and AuxVariables corresponding to "
                             "molalities, etc.  Derived classes will create the solver.");

  return params;
}

AddGeochemistrySolverAction::AddGeochemistrySolverAction(const InputParameters & params)
  : Action(params)
{
}

void
AddGeochemistrySolverAction::act()
{
  if (_current_task == "add_user_object" && isParamValid("execute_console_output_on"))
  {
    const std::string class_name = "NearestNodeNumberUO";
    auto params = _factory.getValidParams(class_name);
    params.set<Point>("point") = getParam<Point>("point");
    if (isParamValid("block"))
      params.set<std::vector<SubdomainName>>("block") =
          getParam<std::vector<SubdomainName>>("block");
    if (isParamValid("boundary"))
      params.set<std::vector<BoundaryName>>("boundary") =
          getParam<std::vector<BoundaryName>>("boundary");
    params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL; // NOTE: adaptivity not active yet
    _problem->addUserObject(class_name, "geochemistry_nearest_node_number", params);
  }
  else if (_current_task == "add_output" && isParamValid("execute_console_output_on"))
  {
    const std::string class_name = "GeochemistryConsoleOutput";
    auto params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("geochemistry_reactor") =
        getParam<UserObjectName>("geochemistry_reactor_name");
    params.set<unsigned>("precision") = getParam<unsigned>("precision");
    params.set<Real>("mol_cutoff") = getParam<Real>("mol_cutoff");
    params.set<Real>("stoichiometry_tolerance") = getParam<Real>("stoichiometry_tolerance");
    params.set<bool>("solver_info") = getParam<bool>("solver_info");
    params.set<UserObjectName>("nearest_node_number_UO") = "geochemistry_nearest_node_number";
    params.set<ExecFlagEnum>("execute_on") = getParam<ExecFlagEnum>("execute_console_output_on");
    _problem->addOutput(class_name, "geochemistry_console_output", params);
  }
  else if (_current_task == "add_geochemistry_molality_aux")
  {
    const ModelGeochemicalDatabase & mgd = _problem
                                               ->getUserObject<GeochemicalModelDefinition>(
                                                   getParam<UserObjectName>("model_definition"))
                                               .getDatabase();
    // add temperature aux, if requested
    if (getParam<bool>("add_aux_temperature"))
      addAuxSpecies("solution_temperature", "H2O", "temperature");
    // add water, if requested
    if (getParam<bool>("add_aux_solvent_kg"))
      addAuxSpecies("kg_solvent_H2O", "H2O", "molal");
    if (getParam<bool>("add_aux_activity"))
      addAuxSpecies("activity_H2O", "H2O", "activity");
    if (getParam<bool>("add_aux_bulk_moles"))
      addAuxSpecies("bulk_moles_H2O", "H2O", "bulk_moles");
    // add pH, if requested
    if (getParam<bool>("add_aux_pH"))
      addAuxSpecies("pH", "H+", "neglog10a");

    // add the remaining ones
    const unsigned num_basis = mgd.basis_species_name.size();
    for (unsigned i = 1; i < num_basis; ++i)
    {
      if (getParam<bool>("add_aux_molal") && !mgd.basis_species_mineral[i])
        addAuxSpecies("molal_" + mgd.basis_species_name[i], mgd.basis_species_name[i], "molal");
      if (getParam<bool>("add_aux_mg_per_kg") && !mgd.basis_species_mineral[i])
        addAuxSpecies(
            "mg_per_kg_" + mgd.basis_species_name[i], mgd.basis_species_name[i], "mg_per_kg");
      if (getParam<bool>("add_aux_free_cm3") && mgd.basis_species_mineral[i])
        addAuxSpecies(
            "free_cm3_" + mgd.basis_species_name[i], mgd.basis_species_name[i], "free_cm3");
      if (getParam<bool>("add_aux_free_mg") && mgd.basis_species_mineral[i])
        addAuxSpecies("free_mg_" + mgd.basis_species_name[i], mgd.basis_species_name[i], "free_mg");
      if (getParam<bool>("add_aux_activity"))
        addAuxSpecies(
            "activity_" + mgd.basis_species_name[i], mgd.basis_species_name[i], "activity");
      if (getParam<bool>("add_aux_bulk_moles"))
        addAuxSpecies(
            "bulk_moles_" + mgd.basis_species_name[i], mgd.basis_species_name[i], "bulk_moles");
    }
    const unsigned num_eqm = mgd.eqm_species_name.size();
    for (unsigned j = 0; j < num_eqm; ++j)
    {
      if (getParam<bool>("add_aux_molal") && !mgd.eqm_species_mineral[j])
        addAuxSpecies("molal_" + mgd.eqm_species_name[j], mgd.eqm_species_name[j], "molal");
      if (getParam<bool>("add_aux_mg_per_kg") && !mgd.eqm_species_mineral[j])
        addAuxSpecies("mg_per_kg_" + mgd.eqm_species_name[j], mgd.eqm_species_name[j], "mg_per_kg");
      if (getParam<bool>("add_aux_free_cm3") && mgd.eqm_species_mineral[j])
        addAuxSpecies("free_cm3_" + mgd.eqm_species_name[j], mgd.eqm_species_name[j], "free_cm3");
      if (getParam<bool>("add_aux_free_mg") && mgd.eqm_species_mineral[j])
        addAuxSpecies("free_mg_" + mgd.eqm_species_name[j], mgd.eqm_species_name[j], "free_mg");
      if (getParam<bool>("add_aux_activity"))
        addAuxSpecies("activity_" + mgd.eqm_species_name[j], mgd.eqm_species_name[j], "activity");
      if (getParam<bool>("add_aux_bulk_moles"))
        addAuxSpecies(
            "bulk_moles_" + mgd.eqm_species_name[j], mgd.eqm_species_name[j], "bulk_moles");
    }
    // add the kinetic aux variables
    const unsigned num_kin = mgd.kin_species_name.size();
    for (unsigned k = 0; k < num_kin; ++k)
    {
      if (getParam<bool>("add_aux_free_cm3") && mgd.kin_species_mineral[k])
        addAuxSpecies("free_cm3_" + mgd.kin_species_name[k], mgd.kin_species_name[k], "free_cm3");
      if (getParam<bool>("add_aux_free_mg") && mgd.kin_species_mineral[k])
        addAuxSpecies("free_mg_" + mgd.kin_species_name[k], mgd.kin_species_name[k], "free_mg");
      if (getParam<bool>("add_aux_kinetic_moles"))
        addAuxSpecies("moles_" + mgd.kin_species_name[k], mgd.kin_species_name[k], "kinetic_moles");
      if (getParam<bool>("add_aux_kinetic_additions"))
        addAuxSpecies(
            "mol_change_" + mgd.kin_species_name[k], mgd.kin_species_name[k], "kinetic_additions");
    }

    // add surface stuff
    for (const auto & mineral : mgd.surface_sorption_name)
    {
      if (getParam<bool>("add_aux_surface_charge"))
        addAuxSpecies("surface_charge_" + mineral, mineral, "surface_charge");
      if (getParam<bool>("add_aux_surface_potential"))
        addAuxSpecies("surface_potential_" + mineral, mineral, "surface_potential");
    }
  }
}

void
AddGeochemistrySolverAction::addAuxSpecies(const std::string & var_name,
                                           const std::string & species_name,
                                           const std::string & quantity)
{
  // add AuxVariable
  auto var_params = _factory.getValidParams("MooseVariable");
  _problem->addAuxVariable("MooseVariable", var_name, var_params);
  // add AuxKernel
  const std::string class_name = "GeochemistryQuantityAux";
  auto params = _factory.getValidParams(class_name);
  params.set<std::string>("species") = species_name;
  params.set<MooseEnum>("quantity") = quantity;
  params.set<UserObjectName>("reactor") = getParam<UserObjectName>("geochemistry_reactor_name");
  params.set<AuxVariableName>("variable") = var_name;
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  _problem->addAuxKernel(class_name, var_name, params);
}
