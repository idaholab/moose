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
  MooseEnum solve_type("PJFNK, JFNK, NEWTON, FD, LINEAR");
  params.addParam<MooseEnum>   ("solve_type",      solve_type,
                                "PJFNK: Preconditioned Jacobian-Free Newton Krylov "
                                "JFNK: Jacobian-Free Newton Krylov "
                                "NEWTON: Full Newton Solve "
                                "FD: Use finite differences to compute Jacobian "
                                "LINEAR: Solving a linear problem");

  // Line Search Options
#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3,3,0)
  MooseEnum line_search("default, cubic, quadratic, none, basic, basicnonorms", "default");
#else
  MooseEnum line_search("default, shell, none, basic, l2, bt, cp", "default");
#endif
  std::string addtl_doc_str(" (Note: none = basic)");
#else
  MooseEnum line_search("default", "default");
  std::string addtl_doc_str("");
#endif
  params.addParam<MooseEnum>   ("line_search",     line_search, "Specifies the line search type" + addtl_doc_str);

#ifdef LIBMESH_HAVE_PETSC
  MooseEnum common_petsc_options("", "", true);
  std::vector<MooseEnum> common_petsc_options_vec(1, common_petsc_options);

  params.addParam<std::vector<MooseEnum> >("petsc_options", common_petsc_options_vec, "Singleton PETSc options");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "Names of PETSc name/value pairs");
  params.addParam<std::vector<std::string> >("petsc_options_value", "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\"");
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

  if (_problem == NULL)
    mooseError("Your simulation contains no \"Problem\" instance.  This might happen if your input file is missing the Mesh section.");

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
  // Note: Options set in the Preconditioner block will override those set in the Executioner block
  if (params.isParamValid("solve_type"))
  {
    // Extract the solve type
    const std::string & solve_type = params.get<MooseEnum>("solve_type");
    fe_problem.solverParams()._type = Moose::stringToEnum<Moose::SolveType>(solve_type);
  }

  MooseEnum line_search = params.get<MooseEnum>("line_search");
  if (fe_problem.solverParams()._line_search == Moose::LS_INVALID || line_search != "default")
    fe_problem.solverParams()._line_search = Moose::stringToEnum<Moose::LineSearchType>(line_search);

#ifdef LIBMESH_HAVE_PETSC
  std::vector<MooseEnum>   petsc_options       = params.get<std::vector<MooseEnum> >  ("petsc_options");
  std::vector<std::string> petsc_options_iname = params.get<std::vector<std::string> >("petsc_options_iname");
  std::vector<std::string> petsc_options_value = params.get<std::vector<std::string> >("petsc_options_value");

  fe_problem.storePetscOptions(petsc_options, petsc_options_iname, petsc_options_value);
#endif //LIBMESH_HAVE_PETSC
}
