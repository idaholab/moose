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
#include "Moose.h"
#include "Parser.h"
#include "Executioner.h"
#include "FEProblem.h"
#include "CoupledProblem.h"
#include "ActionWarehouse.h"

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
  params.addParam<std::string> ("scheme",          "backward-euler",  "Time integration scheme used.");

  //params.addPrivateParam<unsigned int>("steps", 0);  // This is initial adaptivity steps - it'll be set
                                                     // in the adaptivity block later but needs to be here now

#ifdef LIBMESH_HAVE_PETSC
  params.addParam<std::vector<std::string> >("petsc_options", "Singleton Petsc options");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "Names of Petsc name/value pairs");
  params.addParam<std::vector<std::string> >("petsc_options_value", "Values of Petsc name/value pairs (must correspond with \"petsc_options_iname\"");
#endif //LIBMESH_HAVE_PETSC

  return params;
}


CreateExecutionerAction::CreateExecutionerAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
CreateExecutionerAction::act()
{
  //InputParameters class_params = getClassParams();

  // Steady and derived Executioners need to know the number of adaptivity steps to take.  This paramter
  // is held in the child block Adaptivity and needs to be pulled early

#ifdef LIBMESH_HAVE_PETSC
  std::vector<std::string> petsc_options,  petsc_inames, petsc_values;
  petsc_options = getParam<std::vector<std::string> >("petsc_options");
  petsc_inames = getParam<std::vector<std::string> >("petsc_options_iname");
  petsc_values = getParam<std::vector<std::string> >("petsc_options_value");

  Moose::PetscSupport::petscSetOptions(petsc_options, petsc_inames, petsc_values);
#endif //LIBMESH_HAVE_PETSC

  _moose_object_pars.set<MooseMesh *>("_mesh") = _parser_handle._mesh;
  Moose::setup_perf_log.push("Create Executioner","Setup");
  Moose::executioner = static_cast<Executioner *>(Factory::instance()->create(_type, "Executioner", _moose_object_pars));
  Moose::setup_perf_log.pop("Create Executioner","Setup");
  if (dynamic_cast<FEProblem *>(&Moose::executioner->problem()) != NULL)
  {
    FEProblem *mproblem = dynamic_cast<FEProblem *>(&Moose::executioner->problem());
    _parser_handle._problem = mproblem;

    // FIXME: HACK! Can initialize displaced problem after we have instance of problem
    // TODO: Make this into another action
    ActionIterator mesh_it = Moose::action_warehouse.actionBlocksWithActionBegin("read_mesh");
    mooseAssert (mesh_it != Moose::action_warehouse.actionBlocksWithActionEnd("read_mesh"), "No Mesh Block Found!");
    if ((*mesh_it)->isParamValid("displacements"))
    {
      std::vector<std::string> displacements = (*mesh_it)->getParam<std::vector<std::string> >("displacements");
      _parser_handle._problem->initDisplacedProblem(_parser_handle._displaced_mesh, displacements);
    }

    // solver params
    EquationSystems & es = _parser_handle._problem->es();
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
//    _moose_system._no_fe_reinit = getParam<bool>("no_fe_reinit");

    NonlinearSystem & nl = _parser_handle._problem->getNonlinearSystem();
    nl.timeSteppingScheme(Moose::stringToEnum<Moose::TimeSteppingScheme>(getParam<std::string>("scheme")));
  }
}
