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
}

Steady::~Steady()
{
}

void
Steady::execute()
{
  std::cerr << "Time: " << _time << "\n";
  
  checkIntegrity();
  
  _problem.adaptivity().initial();
  // FIXME: move in SubProblem
  //Update the geometric searches (has to be called after the problem is all set up)
  _problem.geomSearchData().update();

  preExecute();

  // FIXME: for backward compatibility
  _problem.computePostprocessors();

  if (_output_initial)
  {
    _problem.output();
    _problem.outputPostprocessors();
  }
  _time = 1.0;           // should be inside the previous if-statement, but to preserve backward compatible behavior, it has to be like this ;(

  // Define the refinement loop
  unsigned int steps = _problem.adaptivity().getSteps();
  for(unsigned int r_step=0; r_step<=steps; r_step++)
  { 
    preSolve();
    _problem.updateMaterials();
    _problem.solve();
    postSolve();

    // TODO: check if the solve converged
    _problem.computePostprocessors();
    _problem.output();
    _problem.outputPostprocessors();

    _problem.getNonlinearSystem().printVarNorms();

    std::cout << "\n";
    
    if(r_step != steps)
    {
      _problem.adaptMesh();
      _problem.out().meshChanged();
      // TODO: This should come out automatically when meshChanged is called
      _problem.mesh().print_info();
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
