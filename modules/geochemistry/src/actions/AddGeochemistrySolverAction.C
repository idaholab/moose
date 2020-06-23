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

registerMooseAction("GeochemistryApp", AddGeochemistrySolverAction, "add_output");
registerMooseAction("GeochemistryApp",
                    AddGeochemistrySolverAction,
                    "add_geochemistry_molality_aux");

InputParameters
AddGeochemistrySolverAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addRequiredParam<UserObjectName>("model_definition",
                                          "The name of the GeochemicalModelDefinition user object "
                                          "(you must create this UserObject yourself)");
  params.addParam<UserObjectName>(
      "geochemistry_reactor_name",
      "geochemistry_reactor",
      "The name that will be given to the GeochemistryReactor UserObject built by this action");
  params.addParam<std::vector<std::string>>(
      "swap_out_of_basis",
      "Species that should be removed from the model_definition's basis and be replaced with the "
      "swap_into_basis species");
  params.addParam<std::vector<std::string>>(
      "swap_into_basis",
      "Species that should be removed from the model_definition's equilibrium species list and "
      "added to the basis.  There must be the same number of species in swap_out_of_basis and "
      "swap_into_basis.  These swaps are performed before any other computations during the "
      "initial problem setup. If this list contains more than one species, the swapping is "
      "performed one-by-one, starting with the first pair (swap_out_of_basis[0] and "
      "swap_into_basis[0]), then the next pair, etc");
  MultiMooseEnum constraint_meaning("moles_bulk_water kg_solvent_water moles_bulk_species "
                                    "free_molality free_moles_mineral_species fugacity activity");
  params.addRequiredParam<MultiMooseEnum>(
      "constraint_meaning",
      constraint_meaning,
      "Meanings of the numerical values given in constraint_value");
  params.addRequiredParam<std::vector<std::string>>(
      "constraint_species",
      "Names of the species that have their values fixed to constraint_value with meaning "
      "constraint_meaning.  All basis species (after swap_into_basis and swap_out_of_basis) must "
      "be provided with exactly one constraint.  These constraints are used to compute the "
      "configuration during the initial problem setup, and in time-dependent simulations they may "
      "be modified as time progresses.");
  params.addRequiredParam<std::vector<Real>>(
      "constraint_value", "Numerical value of the containts on constraint_species");
  params.addRangeCheckedParam<Real>(
      "max_ionic_strength", 3.0, "max_ionic_strength >= 0.0", "Maximum value of ionic strength");
  params.addParam<unsigned>(
      "extra_iterations_to_make_consistent",
      0,
      "Extra iterations to make the molalities, activities, etc, consistent "
      "before commencing the Newton process to find the aqueous configuration");
  params.addRangeCheckedParam<Real>(
      "stoichiometry_tolerance",
      1E-6,
      "stoichiometry_tolerance >= 0.0",
      "Swapping involves inverting matrices via a singular value decomposition. During this "
      "process: (1) if abs(singular value) < stoi_tol * L1norm(singular values), then the "
      "matrix is deemed singular (so the basis swap is deemed invalid); (2) if abs(any "
      "stoichiometric coefficient) < stoi_tol then it is set to zero.");
  params.addRequiredParam<std::string>(
      "charge_balance_species",
      "Charge balance will be enforced on this basis species.  This means that its bulk mole "
      "number may be changed from the initial value you provide in order to ensure charge "
      "neutrality.  After the initial swaps have been performed, this must be in the basis, and it "
      "must be provided with a moles_bulk_species constraint_meaning.");
  params.addParam<std::vector<std::string>>(
      "prevent_precipitation",
      "Mineral species in this list will be prevented from precipitating, irrespective of their "
      "saturation index, unless they are in the basis");
  params.addParam<Real>(
      "abs_tol",
      1E-10,
      "If the residual of the algebraic system (measured in mol) is lower than this value, the "
      "Newton process (that finds the aqueous configuration) is deemed to have converged");
  params.addParam<Real>("rel_tol",
                        1E-200,
                        "If the residual of the algebraic system (measured in mol) is lower than "
                        "this value times the initial residual, the Newton process (that finds the "
                        "aqueous configuration) is deemed to have converged");
  params.addRangeCheckedParam<Real>("min_initial_molality",
                                    1E-20,
                                    "min_initial_molality > 0.0",
                                    "Minimum value of the initial-guess molality used in the "
                                    "Newton process to find the aqueous configuration");
  params.addParam<unsigned>(
      "max_iter",
      100,
      "Maximum number of Newton iterations allowed when finding the aqueous configuration");
  params.addParam<Real>(
      "max_initial_residual",
      1E3,
      "Attempt to alter the initial-guess molalities so that the initial residual for the Newton "
      "process (that finds the aqueous configuration) is less than this number of moles");
  params.addRangeCheckedParam<Real>(
      "swap_threshold",
      0.1,
      "swap_threshold >= 0.0",
      "If the molality of a basis species in the algebraic system falls below swap_threshold * "
      "abs_tol then it is swapped out of the basis.  The dimensions of swap_threshold are "
      "1/kg(solvent water)");
  params.addParam<unsigned>(
      "max_swaps_allowed",
      20,
      "Maximum number of swaps allowed during a single attempt at finding the aqueous "
      "configuration.  Usually only a handful of swaps are used: this parameter prevents endless "
      "cyclic swapping that prevents the algorithm from progressing");
  params.addParam<unsigned>(
      "ramp_max_ionic_strength_initial",
      20,
      "The number of iterations over which to progressively increase the maximum ionic strength "
      "(from zero to max_ionic_strength) during the initial equilibration.  Increasing this can "
      "help in convergence of the Newton process, at the cost of spending more time finding the "
      "aqueous configuration.");
  params.addParam<bool>(
      "ionic_str_using_basis_only",
      false,
      "If set to true, ionic strength and stoichiometric ionic strength will be computed using "
      "only the basis molalities, ignoring molalities of equilibrium species.  Since basis "
      "molality is usually greater than equilibrium molality, and the whole Debye-Huckel concept "
      "of activity coefficients depending on ionic strength is only approximate in practice, "
      "setting this parameter true often results in a reasonable approximation.  It can aid in "
      "convergence since it eliminates problems associated with unphysical huge equilibrium "
      "molalities that can occur during Newton iteration to the solution");

  // following are exclusively for the GeochemistryConsoleOutput
  params.addParam<unsigned int>("precision", 4, "Precision for printing values to the console");
  params.addParam<Real>(
      "mol_cutoff",
      1E-40,
      "Information regarding species with molalities less than this amount will not be outputted");
  params.addParam<bool>("solver_info",
                        false,
                        "Print information (to the console via the geochemistry console output "
                        "object) from the solver including residuals, swaps, etc");
  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum = {EXEC_INITIAL, EXEC_FINAL};
  params.addParam<ExecFlagEnum>(
      "execute_console_output_on", exec_enum, "When to execute the geochemistry console output");

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

AddGeochemistrySolverAction::AddGeochemistrySolverAction(InputParameters params) : Action(params) {}

void
AddGeochemistrySolverAction::act()
{
  if (_current_task == "add_output" && isParamValid("execute_console_output_on"))
  {
    const std::string class_name = "GeochemistryConsoleOutput";
    auto params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("geochemistry_reactor") =
        getParam<UserObjectName>("geochemistry_reactor_name");
    params.set<unsigned>("precision") = getParam<unsigned>("precision");
    params.set<Real>("mol_cutoff") = getParam<Real>("mol_cutoff");
    params.set<Real>("stoichiometry_tolerance") = getParam<Real>("stoichiometry_tolerance");
    params.set<bool>("solver_info") = getParam<bool>("solver_info");
    params.set<Point>("point") = Point(0.0, 0.0, 0.0);
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
