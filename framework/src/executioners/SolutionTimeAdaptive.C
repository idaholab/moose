#include "SolutionTimeAdaptive.h"

//Moose includes
#include "Kernel.h"
#include "ComputeJacobian.h"
#include "ComputeResidual.h"
#include "MaterialFactory.h"

//libMesh includes
#include "implicit_system.h"
#include "nonlinear_implicit_system.h"
#include "transient_system.h"
#include "numeric_vector.h"

// C++ Includes
#include <iomanip>

template<>
InputParameters validParams<SolutionTimeAdaptive>()
{
  InputParameters params = validParams<TransientExecutioner>();

  params.addParam<Real>("percent_change", 0.1, "Percentage to change the timestep by.  Should be between 0 and 1");
  params.addParam<int>("initial_direction", 1, "Direction for the first step.  1 for up... -1 for down. ");
  return params;
}

SolutionTimeAdaptive::SolutionTimeAdaptive(std::string name, InputParameters parameters)
  :TransientExecutioner(name, parameters),
   _percent_change(parameters.get<Real>("percent_change")),
   _direction(parameters.get<int>("initial_direction")),
   _old_sol_time_vs_dt(9999),
   _older_sol_time_vs_dt(9999),
   _sol_time_vs_dt(9999)
{}

void
SolutionTimeAdaptive::preSolve()
{
  gettimeofday (&_solve_start, NULL);
}

void
SolutionTimeAdaptive::postSolve()
{
  if(lastSolveConverged())
  {
    gettimeofday (&_solve_end, NULL);
    double elapsed_time = (static_cast<double>(_solve_end.tv_sec  - _solve_start.tv_sec) +
                           static_cast<double>(_solve_end.tv_usec - _solve_start.tv_usec)*1.e-6);

    _older_sol_time_vs_dt = _old_sol_time_vs_dt;
    _old_sol_time_vs_dt = _sol_time_vs_dt;
    _sol_time_vs_dt = elapsed_time / _dt;
  }
}

Real
SolutionTimeAdaptive::computeDT()
{
  if(!lastSolveConverged())
  {
    std::cout<<"Solve failed... cutting timestep"<<std::endl;
    return _dt / 2.0;
  }

  //Ratio grew so switch direction
  if(_sol_time_vs_dt > _old_sol_time_vs_dt && _sol_time_vs_dt > _older_sol_time_vs_dt)
  {
    _direction *= -1;

    // Make sure we take at least two steps in this new direction
    _old_sol_time_vs_dt = 9999;
    _older_sol_time_vs_dt = 9999;    
  }
  
  return _dt + _dt * _percent_change * _direction;
}
