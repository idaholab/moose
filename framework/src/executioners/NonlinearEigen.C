//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonlinearEigen.h"

registerMooseObject("MooseApp", NonlinearEigen);

defineLegacyParams(NonlinearEigen);

InputParameters
NonlinearEigen::validParams()
{
  InputParameters params = EigenExecutionerBase::validParams();
  params.addParam<unsigned int>("free_power_iterations", 4, "The number of free power iterations");
  params.addDeprecatedParam<Real>("source_abs_tol",
                                  1e-06,
                                  "Absolute tolernance on residual norm",
                                  "Please use nl_abs_tol instead");
  params.set<Real>("nl_abs_tol", true) = 1.0e-06;
  params.addDeprecatedParam<Real>(
      "source_rel_tol",
      1e-50,
      "Relative tolernance on residual norm after free power iterations",
      "Please use nl_rel_tol instead");
  params.set<Real>("nl_rel_tol", true) = 1e-50;
  params.addDeprecatedParam<Real>(
      "pfactor",
      1e-2,
      "The factor of residual to be reduced per free power iteration or per nonlinear step",
      "Please use l_tol");
  params.set<Real>("l_tol", true) = 1e-2;
  params.addParam<Real>("free_l_tol", 1e-2, "Relative linear tolerance in free power iteration");
  params.addParam<bool>(
      "output_after_power_iterations", true, "True to output solution after free power iterations");
  return params;
}

NonlinearEigen::NonlinearEigen(const InputParameters & parameters)
  : EigenExecutionerBase(parameters),
    _free_iter(getParam<unsigned int>("free_power_iterations")),
    _nl_abs_tol(parameters.isParamSetByUser("nl_abs_tol") ? getParam<Real>("nl_abs_tol")
                                                          : getParam<Real>("source_abs_tol")),
    _nl_rel_tol(parameters.isParamSetByUser("nl_rel_tol") ? getParam<Real>("nl_rel_tol")
                                                          : getParam<Real>("source_rel_tol")),
    _l_tol(parameters.isParamSetByUser("l_tol") ? getParam<Real>("l_tol")
                                                : getParam<Real>("pfactor")),
    _free_l_tol(parameters.isParamSetByUser("free_l_tol") ? getParam<Real>("free_l_tol")
                                                          : getParam<Real>("pfactor")),
    _output_after_pi(getParam<bool>("output_after_power_iterations"))
{
  // Delete these silly codes once Rattlesnake is updated
  _abs_tol = _nl_abs_tol, _rel_tol = _nl_rel_tol, _pfactor = _l_tol;
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

  // Write the initial.
  // Note: We need to tempararily change the system time to make the output system work properly.
  _problem.timeStep() = 0;
  Real t = _problem.time();
  _problem.time() = _problem.timeStep();
  _problem.outputStep(EXEC_INITIAL);
  _problem.time() = t;

  if (_free_iter > 0)
  {
    // save the initial guess
    _problem.advanceState();

    // free power iterations
    _console << " Free power iteration starts" << std::endl;

    Real initial_res;
    inversePowerIteration(_free_iter,
                          _free_iter,
                          _free_l_tol,
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

  _last_solve_converged = nonlinearSolve(_nl_rel_tol, _nl_abs_tol, _l_tol, _eigenvalue);
  postSolve();

  if (lastSolveConverged())
  {
    printEigenvalue();

    _problem.onTimestepEnd();
    _problem.execute(EXEC_TIMESTEP_END);
  }
}
