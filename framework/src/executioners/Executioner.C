//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "Executioner.h"

#include "FEProblem.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"
#include "SlepcSupport.h"

// C++ includes
#include <vector>
#include <limits>

template <>
InputParameters
validParams<Executioner>()
{
  InputParameters params = validParams<MooseObject>();
  params.addDeprecatedParam<FileNameNoExtension>(
      "restart_file_base",
      "",
      "File base name used for restart",
      "Please use \"Problem/restart_file_base\" instead");

  params.registerBase("Executioner");

  params.addParamNamesToGroup("restart_file_base", "Restart");

  params.addParam<std::vector<std::string>>("splitting",
                                            "Top-level splitting defining a "
                                            "hierarchical decomposition into "
                                            "subsystems to help the solver.");

  std::set<std::string> line_searches = {"contact", "default", "none", "basic"};
#ifdef LIBMESH_HAVE_PETSC
  std::set<std::string> petsc_line_searches = Moose::PetscSupport::getPetscValidLineSearches();
  line_searches.insert(petsc_line_searches.begin(), petsc_line_searches.end());
#endif // LIBMESH_HAVE_PETSC
  std::string line_search_string = Moose::stringify(line_searches, " ");
  MooseEnum line_search(line_search_string, "default");
  std::string addtl_doc_str(" (Note: none = basic)");
  params.addParam<MooseEnum>(
      "line_search", line_search, "Specifies the line search type" + addtl_doc_str);
  MooseEnum line_search_package("petsc moose", "petsc");
  params.addParam<MooseEnum>("line_search_package",
                             line_search_package,
                             "The solver package to use to conduct the line-search");
  params.addParam<unsigned>("contact_line_search_allowed_lambda_cuts",
                            2,
                            "The number of times lambda is allowed to be cut in half in the "
                            "contact line search. We recommend this number be roughly bounded by 0 "
                            "<= allowed_lambda_cuts <= 3");
  params.addParam<Real>("contact_line_search_ltol",
                        "The linear relative tolerance to be used while the contact state is "
                        "changing between non-linear iterations. We recommend that this tolerance "
                        "be looser than the standard linear tolerance");

// Default Solver Behavior
#ifdef LIBMESH_HAVE_PETSC
  params += Moose::PetscSupport::getPetscValidParams();
#endif // LIBMESH_HAVE_PETSC
  params.addParam<Real>("l_tol", 1.0e-5, "Linear Tolerance");
  params.addParam<Real>("l_abs_step_tol", -1, "Linear Absolute Step Tolerance");
  params.addParam<unsigned int>("l_max_its", 10000, "Max Linear Iterations");
  params.addParam<unsigned int>("nl_max_its", 50, "Max Nonlinear Iterations");
  params.addParam<unsigned int>("nl_max_funcs", 10000, "Max Nonlinear solver function evaluations");
  params.addParam<Real>("nl_abs_tol", 1.0e-50, "Nonlinear Absolute Tolerance");
  params.addParam<Real>("nl_rel_tol", 1.0e-8, "Nonlinear Relative Tolerance");
  params.addParam<Real>("nl_abs_step_tol", 1.0e-50, "Nonlinear Absolute step Tolerance");
  params.addParam<Real>("nl_rel_step_tol", 1.0e-50, "Nonlinear Relative step Tolerance");
  params.addParam<bool>(
      "snesmf_reuse_base",
      true,
      "Specifies whether or not to reuse the base vector for matrix-free calculation");
  params.addParam<bool>("no_fe_reinit", false, "Specifies whether or not to reinitialize FEs");
  params.addParam<bool>("compute_initial_residual_before_preset_bcs",
                        false,
                        "Use the residual norm computed *before* PresetBCs are imposed in relative "
                        "convergence check");

  params.addParam<unsigned int>(
      "picard_max_its",
      1,
      "Maximum number of times each timestep will be solved.  Mainly used when "
      "wanting to do Picard iterations with MultiApps that are set to "
      "execute_on timestep_end or timestep_begin. Setting this parameter to 1 turns off the Picard "
      "iterations.");
  params.addParam<Real>("picard_rel_tol",
                        1e-8,
                        "The relative nonlinear residual drop to shoot for "
                        "during Picard iterations.  This check is "
                        "performed based on the Master app's nonlinear "
                        "residual.");
  params.addParam<Real>("picard_abs_tol",
                        1e-50,
                        "The absolute nonlinear residual to shoot for "
                        "during Picard iterations.  This check is "
                        "performed based on the Master app's nonlinear "
                        "residual.");
  params.addParam<bool>(
      "picard_force_norms",
      false,
      "Force the evaluation of both the TIMESTEP_BEGIN and TIMESTEP_END norms regardless of the "
      "existance of active MultiApps with those execute_on flags, default: false.");

  params.addRangeCheckedParam<Real>("relaxation_factor",
                                    1.0,
                                    "relaxation_factor>0 & relaxation_factor<2",
                                    "Fraction of newly computed value to keep."
                                    "Set between 0 and 2.");
  params.addParam<std::vector<std::string>>("relaxed_variables",
                                            std::vector<std::string>(),
                                            "List of variables to relax during Picard Iteration");

  params.addParamNamesToGroup("picard_max_its picard_rel_tol picard_abs_tol picard_force_norms "
                              "relaxation_factor relaxed_variables",
                              "Picard");

  params.addParam<unsigned int>(
      "max_xfem_update",
      std::numeric_limits<unsigned int>::max(),
      "Maximum number of times to update XFEM crack topology in a step due to evolving cracks");
  params.addParam<bool>("update_xfem_at_timestep_begin",
                        false,
                        "Should XFEM update the mesh at the beginning of the timestep");

  params.addParam<bool>("hypre_matrix",
                        false,
                        "If true, use a hypre matrix to remove the duplicate matrix in PETSc."
                        "It is only active when using Hypre preconditioner");

  params.addParamNamesToGroup("l_tol l_abs_step_tol l_max_its nl_max_its nl_max_funcs "
                              "nl_abs_tol nl_rel_tol nl_abs_step_tol nl_rel_step_tol "
                              "compute_initial_residual_before_preset_bcs",
                              "Solver");
  params.addParamNamesToGroup("no_fe_reinit", "Advanced");

  return params;
}

Executioner::Executioner(const InputParameters & parameters)
  : MooseObject(parameters),
    UserObjectInterface(this),
    PostprocessorInterface(this),
    Restartable(this, "Executioners"),
    PerfGraphInterface(this),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>(
        "_fe_problem_base", "This might happen if you don't have a mesh")),
    _picard_solve(this),
    _initial_residual_norm(std::numeric_limits<Real>::max()),
    _old_initial_residual_norm(std::numeric_limits<Real>::max()),
    _restart_file_base(getParam<FileNameNoExtension>("restart_file_base")),
    _splitting(getParam<std::vector<std::string>>("splitting"))
{
  if (_pars.isParamSetByUser("line_search"))
    _fe_problem.addLineSearch(_pars);

// Extract and store PETSc related settings on FEProblemBase
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::storePetscOptions(_fe_problem, _pars);
#endif // LIBMESH_HAVE_PETSC

  // solver params
  EquationSystems & es = _fe_problem.es();
  es.parameters.set<Real>("linear solver tolerance") = getParam<Real>("l_tol");

  es.parameters.set<Real>("linear solver absolute step tolerance") =
      getParam<Real>("l_abs_step_tol");

  es.parameters.set<unsigned int>("linear solver maximum iterations") =
      getParam<unsigned int>("l_max_its");

  es.parameters.set<unsigned int>("nonlinear solver maximum iterations") =
      getParam<unsigned int>("nl_max_its");

  es.parameters.set<unsigned int>("nonlinear solver maximum function evaluations") =
      getParam<unsigned int>("nl_max_funcs");

  es.parameters.set<Real>("nonlinear solver absolute residual tolerance") =
      getParam<Real>("nl_abs_tol");

  es.parameters.set<Real>("nonlinear solver relative residual tolerance") =
      getParam<Real>("nl_rel_tol");

  es.parameters.set<Real>("nonlinear solver absolute step tolerance") =
      getParam<Real>("nl_abs_step_tol");

  es.parameters.set<Real>("nonlinear solver relative step tolerance") =
      getParam<Real>("nl_rel_step_tol");

  _fe_problem.getNonlinearSystemBase()._compute_initial_residual_before_preset_bcs =
      getParam<bool>("compute_initial_residual_before_preset_bcs");

  _fe_problem.getNonlinearSystemBase()._l_abs_step_tol = getParam<Real>("l_abs_step_tol");

  _fe_problem.setSNESMFReuseBase(getParam<bool>("snesmf_reuse_base"),
                                 parameters.isParamSetByUser("snesmf_reuse_base"));

  if (getParam<Real>("relaxation_factor") != 1.0)
    // Store a copy of the previous solution here
    _fe_problem.getNonlinearSystemBase().addVector("relax_previous", false, PARALLEL);

  _fe_problem.hypreMatrix(getParam<bool>("hypre_matrix"));
}

Executioner::~Executioner() {}

void
Executioner::init()
{
}

void
Executioner::preExecute()
{
}

void
Executioner::postExecute()
{
}

void
Executioner::preSolve()
{
}

void
Executioner::postSolve()
{
}

Problem &
Executioner::problem()
{
  mooseDoOnce(mooseWarning("This method is deprecated, use feProblem() instead"));
  return _fe_problem;
}

FEProblemBase &
Executioner::feProblem()
{
  return _fe_problem;
}

std::string
Executioner::getTimeStepperName()
{
  return std::string();
}

void
Executioner::addAttributeReporter(const std::string & name,
                                  Real & attribute,
                                  const std::string execute_on)
{
  FEProblemBase * problem = getCheckedPointerParam<FEProblemBase *>(
      "_fe_problem_base",
      "Failed to retrieve FEProblemBase when adding a attribute reporter in Executioner");
  InputParameters params = _app.getFactory().getValidParams("ExecutionerAttributeReporter");
  params.set<Real *>("value") = &attribute;
  if (!execute_on.empty())
    params.set<ExecFlagEnum>("execute_on") = execute_on;
  problem->addPostprocessor("ExecutionerAttributeReporter", name, params);
}
