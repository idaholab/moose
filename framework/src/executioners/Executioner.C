/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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

// Default Solver Behavior
#ifdef LIBMESH_HAVE_PETSC
  params += Moose::PetscSupport::getPetscValidParams();
#endif // LIBMESH_HAVE_PETSC
// Add slepc options
#ifdef LIBMESH_HAVE_SLEPC
  params += Moose::SlepcSupport::getSlepcValidParams();
#endif
  params.addParam<Real>("l_tol", 1.0e-5, "Linear Tolerance");
  params.addParam<Real>("l_abs_step_tol", -1, "Linear Absolute Step Tolerance");
  params.addParam<unsigned int>("l_max_its", 10000, "Max Linear Iterations");
  params.addParam<unsigned int>("nl_max_its", 50, "Max Nonlinear Iterations");
  params.addParam<unsigned int>("nl_max_funcs", 10000, "Max Nonlinear solver function evaluations");
  params.addParam<Real>("nl_abs_tol", 1.0e-50, "Nonlinear Absolute Tolerance");
  params.addParam<Real>("nl_rel_tol", 1.0e-8, "Nonlinear Relative Tolerance");
  params.addParam<Real>("nl_abs_step_tol", 1.0e-50, "Nonlinear Absolute step Tolerance");
  params.addParam<Real>("nl_rel_step_tol", 1.0e-50, "Nonlinear Relative step Tolerance");
  params.addParam<bool>("no_fe_reinit", false, "Specifies whether or not to reinitialize FEs");
  params.addParam<bool>("compute_initial_residual_before_preset_bcs",
                        false,
                        "Use the residual norm computed *before* PresetBCs are imposed in relative "
                        "convergence check");

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
    Restartable(parameters, "Executioners"),
    _fe_problem(*parameters.getCheckedPointerParam<FEProblemBase *>(
        "_fe_problem_base", "This might happen if you don't have a mesh")),
    _initial_residual_norm(std::numeric_limits<Real>::max()),
    _old_initial_residual_norm(std::numeric_limits<Real>::max()),
    _restart_file_base(getParam<FileNameNoExtension>("restart_file_base")),
    _splitting(getParam<std::vector<std::string>>("splitting"))
{
// Extract and store PETSc related settings on FEProblemBase
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::storePetscOptions(_fe_problem, _pars);
#endif // LIBMESH_HAVE_PETSC

// Extract and store SLEPc options
#ifdef LIBMESH_HAVE_SLEPC
  Moose::SlepcSupport::storeSlepcOptions(_fe_problem, _pars);
#endif // LIBMESH_HAVE_SLEPC

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

bool
Executioner::lastSolveConverged()
{
  return _fe_problem.converged();
}

void
Executioner::addAttributeReporter(const std::string & name,
                                  Real & attribute,
                                  const std::string execute_on)
{
  FEProblemBase * problem = parameters().getCheckedPointerParam<FEProblemBase *>(
      "_fe_problem_base",
      "Failed to retrieve FEProblemBase when adding a attribute reporter in Executioner");
  InputParameters params = _app.getFactory().getValidParams("ExecutionerAttributeReporter");
  params.set<Real *>("value") = &attribute;
  if (!execute_on.empty())
    params.set<MultiMooseEnum>("execute_on") = execute_on;
  problem->addPostprocessor("ExecutionerAttributeReporter", name, params);
}
