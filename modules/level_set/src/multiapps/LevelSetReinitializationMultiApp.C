/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LevelSetReinitializationMultiApp.h"
#include "LevelSetReinitializationProblem.h"

#include "Executioner.h"

// libMesh
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<LevelSetReinitializationMultiApp>()
{
  InputParameters params = validParams<MultiApp>();
  params.addClassDescription("MultiApp capable of performing repeated complete solves for level set reinitialization.");
  params.addParam<unsigned int>("interval", 1, "Time step interval when to perform reinitialization.");

  params.suppressParameter<std::vector<Point> >("positions");
  params.suppressParameter<std::vector<FileName> >("positions_file");
  params.suppressParameter<bool>("output_in_position");
  params.suppressParameter<Real>("reset_time");
  params.suppressParameter<std::vector<unsigned int> >("reset_apps");
  params.suppressParameter<Real>("move_time");
  params.suppressParameter<std::vector<unsigned int> >("move_apps");
  params.suppressParameter<std::vector<Point> >("move_positions");

  return params;
}


LevelSetReinitializationMultiApp::LevelSetReinitializationMultiApp(const InputParameters & parameters):
    MultiApp(parameters),
    _level_set_problem(NULL),
    _interval(getParam<unsigned int>("interval"))
{
}

void
LevelSetReinitializationMultiApp::initialSetup()
{
  MultiApp::initialSetup();

  if (_has_an_app)
  {
    Executioner * ex = _apps[0]->getExecutioner();

    if (!ex)
      mooseError2("Executioner does not exist!");

    ex->init();

    _executioner = ex;

    _level_set_problem = dynamic_cast<LevelSetReinitializationProblem *>(&appProblemBase(0));
    if (!_level_set_problem)
      mooseError2("The Problem type must be LevelSetReinitializationProblem.");
  }
}

bool
LevelSetReinitializationMultiApp::solveStep(Real /*dt*/, Real /*target_time*/, bool /*auto_advance*/)
{
  // Do nothing if not on interval
  if ((_fe_problem.timeStep() % _interval) != 0)
    return true;

  if (!_has_an_app)
    return true;

  _console << "Solving Reinitialization problem." << std::endl;

  int rank;
  int ierr;
  ierr = MPI_Comm_rank(_orig_comm, &rank);
  mooseCheckMPIErr(ierr);

  bool last_solve_converged = true;

  _level_set_problem->resetTime();
  _executioner->execute();
  if (!_executioner->lastSolveConverged())
    last_solve_converged = false;

  return last_solve_converged;
}
