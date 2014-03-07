/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "FunctionControlledDT.h"
#include "FEProblem.h"
#include "Transient.h"
#include "SubProblem.h"

#include <algorithm> // std::max and std::min

template<>
InputParameters validParams<FunctionControlledDT>()
{
  InputParameters params = validParams<SolutionTimeAdaptiveDT>();
  params.addParam<std::vector<std::string> >("functions", "Name of the functions that will control the time stepping (as a space-separated string).  These functions may be time-dependent, but shouldn't be spatially dependent");
  params.addParam<std::vector<Real> >("maximums", "The maximum values of the functions (as a space-separated string).  If any of the functions exceed their maximum values then the time-step is multiplied by decrement");
  params.addParam<std::vector<Real> >("minimums", "The minimum values of the functions (as a space-separated string).  If all of the functions are less than their minimum values then the time-step is multiplied by increment");
  params.addRequiredParam<Real>("decrement", "The decrement value.  If the value of any of the functions exceed their maximum value, the time-step is multiplied by this amount.  Should be < 1.");
  params.addRequiredParam<Real>("increment", "The increment value.  If the values of all the functions are less than their minimum values then the time step is multiplied by this amount.  Should be > 1.");
  params.addRequiredParam<Real>("minDt", "Miniminum time-step size, below which the time-step will never go");
  params.addRequiredParam<Real>("maxDt", "Maximinum time-step size, above which the time-step will never go");
  params.addClassDescription("This timestepper alters the timestep size based on the walltime needed for a step, and the values of functions.  It acts in the following way\n  (1)SolutionTimeAdaptiveDT timestepper is used to modify dt based on walltime needed for this step and the last two steps.  This timestepper is attempting to minimise the simulation walltime\n  (2) If PETSc cutback on previous time-step then multiply dt by decrement and return\n  (3)If any of the functions are greater than their specified maximum values then multiply dt by decrement and return\n  (4)If all functions are less than their specified minimum values then multiply dt by increment and return\n  (5)Otherwise leave dt at its value given by SolutionTimeAdaptiveDT and return.\nCaveat to above: The timestep size must never lie outside the range minDt and maxDt.");
  return params;
}

FunctionControlledDT::FunctionControlledDT(const std::string & name, InputParameters parameters) :
    SolutionTimeAdaptiveDT(name, parameters),
    FunctionInterface(parameters),
    _maximums(getParam<std::vector<Real> >("maximums")),
    _minimums(getParam<std::vector<Real> >("minimums")),
    _decrement(getParam<Real>("decrement")),
    _increment(getParam<Real>("increment")),
    _cutback_occurred(false),
    _f(),
    _maxdt(getParam<Real>("maxDt")),
    _mindt(getParam<Real>("minDt"))
{
}

FunctionControlledDT::~FunctionControlledDT()
{
}

void
FunctionControlledDT::preSolve()
{
  SolutionTimeAdaptiveDT::preSolve();
}

void
FunctionControlledDT::postSolve()
{
  SolutionTimeAdaptiveDT::postSolve();
}

void
FunctionControlledDT::init()
{
  // have to do this here instead of in constructor because functions haven't been defined when constructor is instantiated
  const std::vector<std::string> & names( getParam<std::vector<std::string> >("functions") );
  const unsigned len( names.size() );
  if (_maximums.size() != len || _minimums.size() != len)
  {
    mooseError("The size of the functions, maximums and minimums vectors must be equal");
  }
  _f.resize(len);
  for (unsigned i(0); i < len; ++i)
  {
    Function * const f = &getFunctionByName( names[i] );
    if (!f)
    {
      std::string msg("Error in FunctionControlledDT.");
      msg += "  Function ";
      msg += names[i];
      msg += " referenced but not found.";
      mooseError( msg );
    }
    _f[i] = f;
  }
}


Real
FunctionControlledDT::computeInitialDT()
{
  return SolutionTimeAdaptiveDT::computeInitialDT();
}

Real
FunctionControlledDT::bounddt(Real tentativedt)
{
  return std::max(std::min(tentativedt, _maxdt), _mindt);
}

Real
FunctionControlledDT::computeDT()
{
  Real local_dt = SolutionTimeAdaptiveDT::computeDT();

  if (_cutback_occurred)
  {
    _cutback_occurred = false;
    return bounddt(local_dt*_decrement);
  }

  // if any functions > maximums then decrease
  for (unsigned i(0); i < _f.size(); ++i)
  {
    if (_f[i]->value(_time, Point(0)) > _maximums[i])
    {
      return bounddt(local_dt*_decrement);
    }
  }

  // if any functions > minimums then don't change
  for (unsigned i(0); i < _f.size(); ++i)
  {
    if (_f[i]->value(_time, Point(0)) > _minimums[i])
    {
      return bounddt(local_dt);
    }
  }

  // all functions must be less than minimums
  return bounddt(local_dt*_increment);

}

void
FunctionControlledDT::rejectStep()
{
  _cutback_occurred = true;
  SolutionTimeAdaptiveDT::rejectStep();
}
