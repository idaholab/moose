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

#include "InversePowerMethod.h"

template<>
InputParameters validParams<InversePowerMethod>()
{
  InputParameters params = validParams<EigenExecutionerBase>();
  params.addParam<PostprocessorName>("xdiff", "To evaluate |x-x_previous| for power iterations");
  params.addParam<unsigned int>("max_power_iterations", 300, "The maximum number of power iterations");
  params.addParam<unsigned int>("min_power_iterations", 1, "Minimum number of power iterations");
  params.addParam<Real>("eig_check_tol", 1e-6, "Eigenvalue convergence tolerance");
  params.addParam<Real>("sol_check_tol", std::numeric_limits<Real>::max(), "|x-x_previous| convergence tolerance");
  params.addParam<Real>("pfactor", 1e-2, "Reduce residual norm per power iteration by this factor");
  params.addParam<bool>("Chebyshev_acceleration_on", true, "If Chebyshev acceleration is turned on");
  params.addParam<Real>("k0", 1.0, "Initial guess of the eigenvalue");
  return params;
}

InversePowerMethod::InversePowerMethod(const std::string & name, InputParameters parameters)
    :EigenExecutionerBase(name, parameters),
     _solution_diff(isParamValid("xdiff") ? &getPostprocessorValue("xdiff") : NULL),
     _min_iter(getParam<unsigned int>("min_power_iterations")),
     _max_iter(getParam<unsigned int>("max_power_iterations")),
     _eig_check_tol(getParam<Real>("eig_check_tol")),
     _sol_check_tol(getParam<Real>("sol_check_tol")),
     _pfactor(getParam<Real>("pfactor")),
     _cheb_on(getParam<bool>("Chebyshev_acceleration_on"))
{
  _eigenvalue = getParam<Real>("k0");
  addRealParameterReporter("eigenvalue");

  if (_max_iter<_min_iter) mooseError("max_power_iterations<min_power_iterations!");
  if (_eig_check_tol<0.0) mooseError("eig_check_tol<0!");
  if (_pfactor<0.0) mooseError("pfactor<0!");
}

void
InversePowerMethod::execute()
{
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
  // we currently do not check the solution difference
  Real initial_res;
  inversePowerIteration(_min_iter, _max_iter, _pfactor, _cheb_on, _eig_check_tol, true,
                        getParam<PostprocessorName>("xdiff"), _sol_check_tol,
                        _eigenvalue, initial_res);
  postSolve();
  printEigenvalue();

  _problem.computeUserObjects(EXEC_TIMESTEP_END, UserObjectWarehouse::PRE_AUX);
  _problem.onTimestepEnd();
  _problem.computeAuxiliaryKernels(EXEC_TIMESTEP_END);
  _problem.computeUserObjects(EXEC_TIMESTEP_END, UserObjectWarehouse::POST_AUX);
}
