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
  }
  _time = 1.0;           // should be inside the previous if-statement, but to preserve backward compatible behavior, it has to be like this ;(

  preSolve();
  _problem.solve();
  postSolve();
  _problem.update();

  // TODO: check if the solve converged
  _problem.computePostprocessors();
  _problem.outputPostprocessors();
  _problem.output();

  postExecute();
}
  
