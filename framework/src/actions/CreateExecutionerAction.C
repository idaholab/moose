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
//  params.addParam<bool>        ("auto_scaling",    false,    "Turns on automatic variable scaling");


  //params.addPrivateParam<unsigned int>("steps", 0);  // This is initial adaptivity steps - it'll be set
                                                     // in the adaptivity block later but needs to be here now

#ifdef LIBMESH_HAVE_PETSC
  MooseEnum common_petsc_options("-ksp_monitor, -snes_mf_operator", "", true);
  std::vector<MooseEnum> common_petsc_options_vec(1, common_petsc_options);

  params.addParam<std::vector<MooseEnum> >("petsc_options", common_petsc_options_vec, "Singleton Petsc options");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "Names of Petsc name/value pairs");
  params.addParam<std::vector<std::string> >("petsc_options_value", "Values of Petsc name/value pairs (must correspond with \"petsc_options_iname\"");
#endif //LIBMESH_HAVE_PETSC

  params.addParamNamesToGroup("l_tol l_abs_step_tol l_max_its nl_max_its nl_max_funcs nl_abs_tol nl_rel_tol nl_abs_step_tol nl_rel_step_tol", "Solver");
  params.addParamNamesToGroup("no_fe_reinit", "Advanced");

  return params;
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

  FEProblem *mproblem = _problem;
  if (mproblem != NULL)
  {
#ifdef LIBMESH_HAVE_PETSC
    mproblem->storePetscOptions(getParam<std::vector<MooseEnum> >("petsc_options"),
                                getParam<std::vector<std::string> >("petsc_options_iname"), getParam<std::vector<std::string> >("petsc_options_value"));
#endif //LIBMESH_HAVE_PETSC

    // solver params
    EquationSystems & es = mproblem->es();
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
    mproblem->getNonlinearSystem()._l_abs_step_tol = getParam<Real>("l_abs_step_tol");
#endif

//    NonlinearSystem & nl = mproblem->getNonlinearSystem();
//    nl.timeSteppingScheme(Moose::stringToEnum<Moose::TimeSteppingScheme>(getParam<MooseEnum>("scheme")));

#ifdef LIBMESH_HAVE_PETSC
    Moose::PetscSupport::petscSetOptions(*_problem);
#endif //LIBMESH_HAVE_PETSC
  }
  _awh.executioner() = executioner;


  /*
  Transient * transient_exec = dynamic_cast<Transient *>(_awh.executioner());

  if (!transient_exec)
    mooseError("Time Periods are not valid for Steady executioners");
  */
}
