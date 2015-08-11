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

#include "CreateExecutionerAction.h"
#include "Factory.h"
#include "PetscSupport.h"
#include "Conversion.h"
#include "MooseApp.h"
#include "Executioner.h"
#include "FEProblem.h"
#include "ActionWarehouse.h"
#include "MooseEnum.h"
#include "MultiMooseEnum.h"

template<>
InputParameters validParams<CreateExecutionerAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.addParam<Real>        ("l_tol",           1.0e-5,   "Linear Tolerance");
  params.addParam<Real>        ("l_abs_step_tol",  -1,       "Linear Absolute Step Tolerance");
  params.addParam<unsigned int>("l_max_its",       10000,    "Max Linear Iterations");
  params.addParam<unsigned int>("nl_max_its",      50,       "Max Nonlinear Iterations");
  params.addParam<unsigned int>("nl_max_funcs",    10000,    "Max Nonlinear solver function evaluations");
  params.addParam<Real>        ("nl_abs_tol",      1.0e-50,  "Nonlinear Absolute Tolerance");
  params.addParam<Real>        ("nl_rel_tol",      1.0e-8,   "Nonlinear Relative Tolerance");
  params.addParam<Real>        ("nl_abs_step_tol", 1.0e-50,  "Nonlinear Absolute step Tolerance");
  params.addParam<Real>        ("nl_rel_step_tol", 1.0e-50,  "Nonlinear Relative step Tolerance");
  params.addParam<bool>        ("no_fe_reinit",    false,    "Specifies whether or not to reinitialize FEs");
  params.addParam<bool>        ("compute_initial_residual_before_preset_bcs", false,
                                "Use the residual norm computed *before* PresetBCs are imposed in relative convergence check");

  // Adds the PETSc related options to the Executioner block
  params += Moose::PetscSupport::getPetscValidParams();

  params.addParamNamesToGroup("l_tol l_abs_step_tol l_max_its nl_max_its nl_max_funcs "
                              "nl_abs_tol nl_rel_tol nl_abs_step_tol nl_rel_step_tol compute_initial_residual_before_preset_bcs", "Solver");
  params.addParamNamesToGroup("no_fe_reinit", "Advanced");

  return params;
}


CreateExecutionerAction::CreateExecutionerAction(InputParameters params) :
    MooseObjectAction(params)
{
}

void
CreateExecutionerAction::act()
{
  // Steady and derived Executioners need to know the number of adaptivity steps to take.  This parameter
  // is held in the child block Adaptivity and needs to be pulled early
  if (_problem.get() != NULL)
  {
    // Extract and store PETSc related settings on FEProblem
#ifdef LIBMESH_HAVE_PETSC
    Moose::PetscSupport::storePetscOptions(*_problem, _pars);
#endif //LIBMESH_HAVE_PETSC

    // solver params
    EquationSystems & es = _problem->es();
    es.parameters.set<Real> ("linear solver tolerance")
      = getParam<Real>("l_tol");

    es.parameters.set<Real> ("linear solver absolute step tolerance")
      = getParam<Real>("l_abs_step_tol");

    es.parameters.set<unsigned int> ("linear solver maximum iterations")
      = getParam<unsigned int>("l_max_its");

    es.parameters.set<unsigned int> ("nonlinear solver maximum iterations")
      = getParam<unsigned int>("nl_max_its");

    es.parameters.set<unsigned int> ("nonlinear solver maximum function evaluations")
      = getParam<unsigned int>("nl_max_funcs");

    es.parameters.set<Real> ("nonlinear solver absolute residual tolerance")
      = getParam<Real>("nl_abs_tol");

    es.parameters.set<Real> ("nonlinear solver relative residual tolerance")
      = getParam<Real>("nl_rel_tol");

    es.parameters.set<Real> ("nonlinear solver absolute step tolerance")
      = getParam<Real>("nl_abs_step_tol");

    es.parameters.set<Real> ("nonlinear solver relative step tolerance")
      = getParam<Real>("nl_rel_step_tol");

#ifdef LIBMESH_HAVE_PETSC
    _problem->getNonlinearSystem()._l_abs_step_tol = getParam<Real>("l_abs_step_tol");
#endif

    _problem->getNonlinearSystem()._compute_initial_residual_before_preset_bcs = getParam<bool>("compute_initial_residual_before_preset_bcs");
  }

  Moose::setup_perf_log.push("Create Executioner","Setup");
  _moose_object_pars.set<FEProblem *>("_fe_problem") = _problem.get();
  MooseSharedPointer<Executioner> executioner = MooseSharedNamespace::static_pointer_cast<Executioner>(_factory.create(_type, "Executioner", _moose_object_pars));
  Moose::setup_perf_log.pop("Create Executioner","Setup");

  _app.executioner() = executioner;
}
