//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InversePowerMethod.h"

registerMooseObject("MooseApp", InversePowerMethod);

InputParameters
InversePowerMethod::validParams()
{
  InputParameters params = EigenExecutionerBase::validParams();
  params.addClassDescription("Inverse power method for eigenvalue problems.");
  params.addParam<PostprocessorName>(
      "xdiff", "", "To evaluate |x-x_previous| for power iterations");
  params.addParam<unsigned int>(
      "max_power_iterations", 300, "The maximum number of power iterations");
  params.addParam<unsigned int>("min_power_iterations", 1, "Minimum number of power iterations");
  params.addParam<Real>("eig_check_tol", 1e-6, "Eigenvalue convergence tolerance");
  params.addParam<Real>("sol_check_tol",
                        std::numeric_limits<Real>::max(),
                        "Convergence tolerance on |x-x_previous| when provided");
  params.set<Real>("l_tol", true) = 1e-2;
  params.addParam<bool>(
      "Chebyshev_acceleration_on", true, "If Chebyshev acceleration is turned on");
  return params;
}

InversePowerMethod::InversePowerMethod(const InputParameters & parameters)
  : EigenExecutionerBase(parameters),
    _solution_diff_name(getParam<PostprocessorName>("xdiff")),
    _min_iter(getParam<unsigned int>("min_power_iterations")),
    _max_iter(getParam<unsigned int>("max_power_iterations")),
    _eig_check_tol(getParam<Real>("eig_check_tol")),
    _sol_check_tol(getParam<Real>("sol_check_tol")),
    _l_tol(getParam<Real>("l_tol")),
    _cheb_on(getParam<bool>("Chebyshev_acceleration_on"))
{
  if (_max_iter < _min_iter)
    mooseError("max_power_iterations<min_power_iterations!");
  if (_eig_check_tol < 0.0)
    mooseError("eig_check_tol<0!");
  if (_l_tol < 0.0)
    paramError("l_tol", "l_tol<0!");

  mooseInfo(
      "'InversePowerMethod' executioner is deprecated in favor of 'Eigenvalue' executioner.\n",
      "Few parameters such as 'bx_norm', 'k0', 'xdiff', 'max_power_iterations', "
      "'min_power_iterations', 'eig_check_tol', 'sol_check_tol', and 'output_before_normalization' "
      "are no longer supported.\n",
      "However, 'Eigenvalue' executioner supports more solving options by interfacing SLEPc.\n");
}

void
InversePowerMethod::init()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover InversePowerMethod solves!\nExiting...\n" << std::endl;
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
}

void
InversePowerMethod::execute()
{
  if (_app.isRecovering())
    return;

  preExecute();

  takeStep();

  postExecute();
}

void
InversePowerMethod::takeStep()
{
  // save the initial guess and mark a new time step
  _problem.advanceState();

  preSolve();
  Real initial_res;
  _last_solve_converged = inversePowerIteration(_min_iter,
                                                _max_iter,
                                                _l_tol,
                                                _cheb_on,
                                                _eig_check_tol,
                                                true,
                                                _solution_diff_name,
                                                _sol_check_tol,
                                                _eigenvalue,
                                                initial_res);
  postSolve();

  if (lastSolveConverged())
  {
    printEigenvalue();
    _problem.onTimestepEnd();
    _problem.execute(EXEC_TIMESTEP_END);
  }
}
