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

  CreateExecutionerAction::populateCommonExecutionerParams(params);

  params.addParamNamesToGroup("l_tol l_abs_step_tol l_max_its nl_max_its nl_max_funcs nl_abs_tol nl_rel_tol nl_abs_step_tol nl_rel_step_tol", "Solver");
  params.addParamNamesToGroup("no_fe_reinit", "Advanced");

  return params;
}

void
CreateExecutionerAction::populateCommonExecutionerParams(InputParameters & params)
{
  MooseEnum solve_type("PJFNK, JFNK, NEWTON, FD");
  params.addParam<MooseEnum>   ("solve_type",      solve_type,
                                "PJFNK: Preconditioned Jacobian-Free Newton Krylov "
                                "JFNK: Jacobian-Free Newton Krylov "
                                "NEWTON: Full Newton Solve"
                                "FD: Use finite differences to compute Jacobian");

#ifdef LIBMESH_HAVE_PETSC

  // Line Search Options
#if PETSC_VERSION_LESS_THAN(3,3,0)
  MooseEnum line_search("default, cubic, quadratic, none, basic, basicnonorms", "default");
#else
  MooseEnum line_search("default, shell, none, basic, l2, bt, cp", "default");
#endif
  params.addParam<MooseEnum>   ("line_search",     line_search, "Specifies the line search type (Note: none = basic)");
  params.addParam<bool>("print_linear_residuals", false, "Specifies whether the linear residuals are printed during the solve");

  MooseEnum common_petsc_options("", "", true);
  std::vector<MooseEnum> common_petsc_options_vec(1, common_petsc_options);

  params.addParam<std::vector<MooseEnum> >("petsc_options", common_petsc_options_vec, "Singleton Petsc options");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "Names of Petsc name/value pairs");
  params.addParam<std::vector<std::string> >("petsc_options_value", "Values of Petsc name/value pairs (must correspond with \"petsc_options_iname\"");
#endif //LIBMESH_HAVE_PETSC
}

CreateExecutionerAction::CreateExecutionerAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
CreateExecutionerAction::act()
{
  // Steady and derived Executioners need to know the number of adaptivity steps to take.  This parameter
  // is held in the child block Adaptivity and needs to be pulled early

  Moose::setup_perf_log.push("Create Executioner","Setup");
  _moose_object_pars.set<FEProblem *>("_fe_problem") = _problem;
  Executioner * executioner = static_cast<Executioner *>(_factory.create(_type, "Executioner", _moose_object_pars));
  Moose::setup_perf_log.pop("Create Executioner","Setup");

  if (_problem != NULL)
  {
    storeCommonExecutionerParams(*_problem, _pars);

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

  }

  _awh.executioner() = executioner;
}


void
CreateExecutionerAction::storeCommonExecutionerParams(FEProblem & fe_problem, InputParameters & params)
{

#ifdef LIBMESH_HAVE_PETSC
  std::vector<MooseEnum>   petsc_options       = params.get<std::vector<MooseEnum> >  ("petsc_options");
  std::vector<std::string> petsc_options_iname = params.get<std::vector<std::string> >("petsc_options_iname");
  std::vector<std::string> petsc_options_value = params.get<std::vector<std::string> >("petsc_options_value");

  if (params.get<bool>("print_linear_residuals"))
    petsc_options.push_back(MooseEnum("-ksp_monitor=5678", "-ksp_monitor"));

  if (params.isParamValid("solve_type"))
  {
    // Extract the solve type
    MooseEnum raw_solve_type = params.get<MooseEnum>("solve_type");

    /**
     *  Here we set the PETSc option with a magic num (5678) so that we won't warn the user later when
     *  we ship the final option to PETSc
     */
    switch (raw_solve_type)
    {
    case 0: // PJFNK
      petsc_options.push_back(MooseEnum("-snes_mf_operator=5678", "-snes_mf_operator")); break;
    case 1: // JFNK
      petsc_options.push_back(MooseEnum("-snes_mf=5678", "-snes_mf")); break;
    case 2: // NEWTON
      petsc_options.push_back(MooseEnum("-newton=5678", "-newton")); break;
    case 3: // FD
      petsc_options.push_back(MooseEnum("-snes_fd=5678", "-snes_fd")); break;
    }
  }

  // Extract the line search options
  MooseEnum line_search = params.get<MooseEnum>("line_search");
  // Change linesearch type of "none" to "basic"
  if (line_search == "none")
    line_search = "basic";

  if (line_search != "default")
  {
#if PETSC_VERSION_LESS_THAN(3,3,0)
    petsc_options_iname.push_back("-snes_type");
    petsc_options_iname.push_back("-snes_ls");
    petsc_options_value.push_back("ls");
#else
    petsc_options_iname.push_back("-snes_linesearch_type");
#endif

    petsc_options_value.push_back(line_search);
  }

  fe_problem.storePetscOptions(petsc_options, petsc_options_iname, petsc_options_value);
#endif //LIBMESH_HAVE_PETSC
}
