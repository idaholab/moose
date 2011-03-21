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
  // FIXME: move in SubProblem
  //Update the geometric searches (has to be called after the problem is all set up)
  _problem._geometric_search_data.update();

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
    _problem.getNonlinearSystem().setScaling();
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
      _problem.adaptMesh();
  }

  postExecute();
}
  
