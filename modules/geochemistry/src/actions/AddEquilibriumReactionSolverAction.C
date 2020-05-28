//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddEquilibriumReactionSolverAction.h"
#include "Executioner.h"
#include "FEProblem.h"

registerMooseAction("GeochemistryApp", AddEquilibriumReactionSolverAction, "setup_mesh");
registerMooseAction("GeochemistryApp", AddEquilibriumReactionSolverAction, "init_mesh");
registerMooseAction("GeochemistryApp", AddEquilibriumReactionSolverAction, "create_problem");
registerMooseAction("GeochemistryApp", AddEquilibriumReactionSolverAction, "setup_executioner");
registerMooseAction("GeochemistryApp", AddEquilibriumReactionSolverAction, "add_output");

InputParameters
AddEquilibriumReactionSolverAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addRequiredParam<UserObjectName>(
      "model_definition",
      "The name of the GeochemicalModelDefinition user object.  Only equilibrium reactions are "
      "solved by this class, so the model_definition must not contain any kinetic species");
  params.addParam<std::vector<std::string>>(
      "swap_out_of_basis",
      "Species that should be removed from the model_definition's basis and be replaced with the "
      "swap_into_basis species.  There must be the same number of species in swap_out_of_basis and "
      "swap_into_basis.  If this list contains more than one species, the swapping is performed "
      "one-by-one, starting with the first pair (swap_out_of_basis[0] and swap_into_basis[0]), "
      "then the next pair, etc");
  params.addParam<std::vector<std::string>>(
      "swap_into_basis",
      "Species that should be removed from the model_definition's "
      "equilibrium species list and added to the basis");
  params.addParam<std::vector<std::string>>(
      "nernst_swap_out_of_basis",
      "Before outputting Nernst potentials (after solving the system) these species are swapped "
      "out of the basis.  Often this is identical to swap_into_basis, so that the Nernst "
      "potentials are defined in terms of the original model definition.  There must be the same "
      "number of species in nernst_swap_out_of_basis and nernst_swap_into_basis.  If this list "
      "contains more than one species, the swapping is performed one-by-one, starting with the "
      "first pair (nernst_swap_out_of_basis[0] and nernst_swap_into_basis[0]) then the next pair, "
      "etc");
  params.addParam<std::vector<std::string>>("nernst_swap_into_basis",
                                            "Before outputting Nernst potentials (after solving "
                                            "the system) these species are swapped into the basis");
  MultiMooseEnum constraint_meaning("moles_bulk_water kg_solvent_water moles_bulk_species "
                                    "free_molality free_moles_mineral_species fugacity activity");
  params.addRequiredParam<MultiMooseEnum>(
      "constraint_meaning",
      constraint_meaning,
      "Meanings of the numerical values given in constrain_value");
  params.addRequiredParam<std::vector<std::string>>(
      "constraint_species",
      "Names of the species that have their values fixed to constraint_value with meaning "
      "constraint_meaning.  All basis species (after swap_into_basis and swap_out_of_basis) must "
      "be provided with exactly one constraint");
  params.addRequiredParam<std::vector<Real>>(
      "constraint_value", "Numerical value of the containts on constraint_species");
  params.addParam<unsigned int>("precision", 4, "Precision for printing values to the console");
  params.addParam<Real>(
      "mol_cutoff",
      1E-40,
      "Information regarding species with molalities less than this amount will not be outputted");
  params.addRangeCheckedParam<Real>(
      "max_ionic_strength", 3.0, "max_ionic_strength >= 0.0", "Maximum value of ionic strength");
  params.addParam<unsigned>("extra_iterations_to_make_consistent",
                            0,
                            "Extra iterations to make the molalities, activities, etc, consistent "
                            "before commencing the solution process's Newton iterations");
  params.addParam<Real>("temperature", 25, "Temperature of the aqueous solution");
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
      "Charge balance will be enforced on this basis species.  After swaps have been performed, "
      "this must be in the basis");
  params.addParam<std::vector<std::string>>(
      "prevent_precipitation",
      "Mineral species in this list will be prevented from precipitating, irrespective of their "
      "saturation index, unless they are in the basis");
  params.addParam<Real>("abs_tol",
                        1E-10,
                        "If the residual of the algebraic system (measured in mol) is lower than "
                        "this value, the Newton process is deemed to have converged");
  params.addParam<Real>("rel_tol",
                        1E-200,
                        "If the residual of the algebraic system is lower than this value times "
                        "the initial residual, the Newton process is deemed to have converged");
  params.addRangeCheckedParam<Real>(
      "min_initial_molality",
      1E-20,
      "min_initial_molality > 0.0",
      "Minimum value of the initial-guess molality used in the Newton process");
  params.addParam<unsigned>(
      "max_iter",
      100,
      "Maximum number of Newton iterations allowed to solve one round of the algebraic system");
  params.addParam<bool>("verbose", false, "Print verbose information");
  params.addParam<Real>(
      "max_initial_residual",
      1E3,
      "Attempt to alter the initial-guess molalities so that the initial residual "
      "for the Newton process is less than this number of moles");
  params.addRangeCheckedParam<Real>(
      "swap_threshold",
      0.1,
      "swap_threshold >= 0.0",
      "If the molality of a basis species in the algebraic system falls below swap_threshold * "
      "abs_tol then it is swapped out of the basis.  The dimensions of swap_threshold are "
      "1/kg(solvent water)");
  params.addParam<unsigned>("ramp_max_ionic_strength",
                            20,
                            "The number of iterations over which to progressively increase the "
                            "maximum ionic strength (from zero to max_ionic_strength).  Increasing "
                            "this can help in convergence of the Newton process");
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
  params.addClassDescription("Action that sets up the equilibrium reaction solver");

  return params;
}

AddEquilibriumReactionSolverAction::AddEquilibriumReactionSolverAction(InputParameters params)
  : Action(params)
{
}

void
AddEquilibriumReactionSolverAction::act()
{
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
  // Set up an arbitrary steady executioner
  else if (_current_task == "setup_executioner")
  {
    const std::string class_name = "Steady";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<FEProblemBase *>("_fe_problem_base") = _problem.get();
    params.set<FEProblem *>("_fe_problem") = (std::dynamic_pointer_cast<FEProblem>(_problem)).get();
    std::shared_ptr<Executioner> executioner =
        _factory.create<Executioner>(class_name, "Executioner", params);
    _app.setExecutioner(std::move(executioner));
  }
  else if (_current_task == "add_output")
  {
    const std::string class_name = "EquilibriumReactionSolverOutput";
    auto params = _factory.getValidParams(class_name);
    // Only pass parameters that were supplied to this action
    params.set<UserObjectName>("model_definition") = getParam<UserObjectName>("model_definition");
    if (isParamValid("swap_out_of_basis"))
      params.set<std::vector<std::string>>("swap_out_of_basis") =
          getParam<std::vector<std::string>>("swap_out_of_basis");
    if (isParamValid("swap_into_basis"))
      params.set<std::vector<std::string>>("swap_into_basis") =
          getParam<std::vector<std::string>>("swap_into_basis");
    if (isParamValid("nernst_swap_out_of_basis"))
      params.set<std::vector<std::string>>("nernst_swap_out_of_basis") =
          getParam<std::vector<std::string>>("nernst_swap_out_of_basis");
    if (isParamValid("nernst_swap_into_basis"))
      params.set<std::vector<std::string>>("nernst_swap_into_basis") =
          getParam<std::vector<std::string>>("nernst_swap_into_basis");
    params.set<MultiMooseEnum>("constraint_meaning") =
        getParam<MultiMooseEnum>("constraint_meaning");
    params.set<std::vector<std::string>>("constraint_species") =
        getParam<std::vector<std::string>>("constraint_species");
    params.set<std::vector<Real>>("constraint_value") =
        getParam<std::vector<Real>>("constraint_value");
    params.set<unsigned>("precision") = getParam<unsigned>("precision");
    params.set<Real>("mol_cutoff") = getParam<Real>("mol_cutoff");
    params.set<Real>("max_ionic_strength") = getParam<Real>("max_ionic_strength");
    params.set<unsigned>("extra_iterations_to_make_consistent") =
        getParam<unsigned>("extra_iterations_to_make_consistent");
    params.set<Real>("temperature") = getParam<Real>("temperature");
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
    params.set<bool>("verbose") = getParam<bool>("verbose");
    params.set<Real>("max_initial_residual") = getParam<Real>("max_initial_residual");
    params.set<Real>("swap_threshold") = getParam<Real>("swap_threshold");
    params.set<unsigned>("ramp_max_ionic_strength") = getParam<unsigned>("ramp_max_ionic_strength");
    params.set<bool>("ionic_str_using_basis_only") = getParam<bool>("ionic_str_using_basis_only");
    _problem->addOutput(class_name, "equilibrium_reaction_solver_output", params);
  }
}
