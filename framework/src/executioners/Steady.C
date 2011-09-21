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

#include "Steady.h"

#include "equation_systems.h"
#include "ProblemFactory.h"

template<>
InputParameters validParams<Steady>()
{
  return validParams<Executioner>();
}


Steady::Steady(const std::string & name, InputParameters parameters) :
    Executioner(name, parameters),
    _problem(*ProblemFactory::instance()->createFEProblem(_mesh)),
    _time_step(_problem.timeStep()),
    _time(_problem.time())
{
  if (!_restart_sln_file_name.empty())
    _problem.setRestartFile(_restart_sln_file_name);
}

Steady::~Steady()
{
  // This problem was built by the Factory and needs to be released by this destructor
  delete &_problem;
}

void
Steady::execute()
{
  std::cout << "Time: " << _time_step << "\n";

  checkIntegrity();

  _problem.initialSetup();

  preExecute();

  // first step in any steady state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;
  _time = _time_step;                 // need to keep _time in sync with _time_step to get correct output

#ifdef LIBMESH_ENABLE_AMR

  // Define the refinement loop
  unsigned int steps = _problem.adaptivity().getSteps();
  for(unsigned int r_step=0; r_step<=steps; r_step++)
  {
#endif //LIBMESH_ENABLE_AMR
    preSolve();
    _problem.updateMaterials();
    _problem.timestepSetup();
    _problem.solve();
    postSolve();

    _problem.computePostprocessors();
    _problem.output();
    _problem.outputPostprocessors();

    _problem.getNonlinearSystem().printVarNorms();

    std::cout << "\n";

#ifdef LIBMESH_ENABLE_AMR
    if(r_step != steps)
    {
      _problem.adaptMesh();
      _problem.out().meshChanged();
    }

    _time_step++;
    _time = _time_step;                 // need to keep _time in sync with _time_step to get correct output
  }
#endif

  postExecute();
}

void
Steady::checkIntegrity()
{
  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  if (_problem.getNonlinearSystem().containsTimeKernel())
    mooseError("You have specified time kernels in your steady state simulation");
}
