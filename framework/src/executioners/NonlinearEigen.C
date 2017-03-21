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

#include "NonlinearEigen.h"

template <>
InputParameters
validParams<NonlinearEigen>()
{
  InputParameters params = validParams<EigenExecutionerBase>();
  params.addParam<unsigned int>("free_power_iterations", 4, "The number of free power iterations");
  params.addParam<Real>("source_abs_tol", 1e-06, "Absolute tolernance on residual norm");
  params.addParam<Real>(
      "source_rel_tol", 1e-50, "Relative tolernance on residual norm after free power iterations");
  params.addParam<Real>(
      "pfactor",
      1e-2,
      "The factor of residual to be reduced per free power iteration or per nonlinear step");
  params.addParam<Real>("k0", 1.0, "Initial guess of the eigenvalue");
  params.addParam<bool>(
      "output_after_power_iterations", true, "True to output solution after free power iterations");
  return params;
}

NonlinearEigen::NonlinearEigen(const InputParameters & parameters)
  : EigenExecutionerBase(parameters),
    _free_iter(getParam<unsigned int>("free_power_iterations")),
    _abs_tol(getParam<Real>("source_abs_tol")),
    _rel_tol(getParam<Real>("source_rel_tol")),
    _pfactor(getParam<Real>("pfactor")),
    _output_after_pi(getParam<bool>("output_after_power_iterations"))
{
  if (!_app.isRecovering() && !_app.isRestarting())
    _eigenvalue = getParam<Real>("k0");

  addAttributeReporter("eigenvalue", _eigenvalue, "initial timestep_end");
}

void
NonlinearEigen::init()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover NonlinearEigen solves!\nExiting...\n" << std::endl;
    return;
  }

  EigenExecutionerBase::init();

  if (_free_iter > 0)
  {
    // save the initial guess
    _problem.advanceState();

    // free power iterations
    _console << " Free power iteration starts" << std::endl;

    Real initial_res;
    inversePowerIteration(_free_iter,
                          _free_iter,
                          _pfactor,
                          false,
                          std::numeric_limits<Real>::min(),
                          true,
                          "",
                          std::numeric_limits<Real>::max(),
                          _eigenvalue,
                          initial_res);

    _problem.onTimestepEnd();
    _problem.execute(EXEC_TIMESTEP_END);

    if (_output_after_pi)
    {
      // output initial guess created by free power iterations
      _problem.timeStep()++;
      Real t = _problem.time();
      _problem.time() = _problem.timeStep();
      _problem.outputStep(EXEC_TIMESTEP_END);
      _problem.time() = t;
    }
  }
}

void
NonlinearEigen::execute()
{
  if (_app.isRecovering())
    return;

  preExecute();

  takeStep();

  postExecute();
}

void
NonlinearEigen::takeStep()
{
  _console << " Nonlinear iteration starts" << std::endl;

  preSolve();
  _problem.timestepSetup();
  _problem.advanceState();
  _problem.execute(EXEC_TIMESTEP_BEGIN);

  nonlinearSolve(_rel_tol, _abs_tol, _pfactor, _eigenvalue);
  postSolve();

  if (lastSolveConverged())
  {
    printEigenvalue();

    _problem.onTimestepEnd();
    _problem.execute(EXEC_TIMESTEP_END);
  }
}
