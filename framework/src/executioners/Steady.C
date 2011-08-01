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

template<>
InputParameters validParams<Steady>()
{
  return validParams<Executioner>();
}


Steady::Steady(const std::string & name, InputParameters parameters) :
    Executioner(name, parameters),
    _problem(*_mesh),
    _time(_problem.time())
{
  if (!_restart_sln_file_name.empty())
    _problem.setRestartFile(_restart_sln_file_name);
}

Steady::~Steady()
{
}

void
Steady::execute()
{
  std::cerr << "Time: " << _time << "\n";
  
  checkIntegrity();

  _problem.initialSetup();
  
  preExecute();

  _time = 1.0;           // should be inside the previous if-statement, but to preserve backward compatible behavior, it has to be like this ;(

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
    _time += 1.0;                       // change the "time" so we get the right output
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
