#include "Steady.h"
#include "Problem.h"

#include "equation_systems.h"

template<>
InputParameters validParams<Steady>()
{
  InputParameters params = validParams<Executioner>();
  return params;
}


Steady::Steady(const std::string & name, InputParameters parameters) :
    Executioner(name, parameters),
    _time(_problem.time())
{
}

void
Steady::execute()
{
  preExecute();
  _problem.update();
  if (_output_initial)
  {
    _problem.output();
    _time += 1.0;
  }

  preSolve();
  _problem.solve();
  postSolve();
  _problem.update();

  // TODO: check if the solve converged
  _problem.output();

  postExecute();
}
  
