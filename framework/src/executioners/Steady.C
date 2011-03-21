#include "Steady.h"

#include "equation_systems.h"

template<>
InputParameters validParams<Steady>()
{
  InputParameters params = validParams<Executioner>();
  return params;
}


Steady::Steady(const std::string & name, InputParameters parameters) :
    Executioner(name, parameters),
    _problem(*_mesh),
    _steps(getParam<unsigned int>("steps")),
    _time(_problem.time())
{
}

Steady::~Steady()
{
}

void
Steady::execute()
{
  checkIntegrity();
  
  _problem.adaptivity().initial();
  // FIXME: move in SubProblem
  //Update the geometric searches (has to be called after the problem is all set up)
  _problem.geomSearchData().update();

  preExecute();
  _problem.update();

  // FIXME: for backward compatibility
  _problem.computePostprocessors();

  if (_output_initial)
  {
    _problem.output();
    _problem.outputPostprocessors();
  }
  _time = 1.0;           // should be inside the previous if-statement, but to preserve backward compatible behavior, it has to be like this ;(

  // Define the refinement loop
  for(unsigned int r_step=0; r_step<=_steps; r_step++)
  {
    preSolve();
    _problem.updateMaterials();
    _problem.solve();
    postSolve();
    _problem.update();

    // TODO: check if the solve converged
    _problem.computePostprocessors();
    _problem.output();
    _problem.outputPostprocessors();

    if(r_step != _steps)
    {
      _problem.adaptMesh();
      _problem.out().meshChanged();
    }
  }

  postExecute();
}
  
void
Steady::checkIntegrity()
{
  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  if (_problem.getNonlinearSystem().containsTimeKernel())
    mooseError("You have specified time kernels in your steady state simulation");
}
