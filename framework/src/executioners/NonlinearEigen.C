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

template<>
InputParameters validParams<NonlinearEigen>()
{
  InputParameters params = validParams<EigenExecutionerBase>();
  params.addParam<unsigned int>("free_power_iterations", 4, "The number of free power iterations");
  params.addParam<Real>("source_abs_tol", 1e-06, "Absolute tolernance on residual norm");
  params.addParam<Real>("source_rel_tol", 1e-50, "Relative tolernance on residual norm");
  params.addParam<Real>("pfactor", 1e-2, "The factor of residual to be reduced per power iteration");
  params.addParam<Real>("k0", 1.0, "Initial guess of the eigenvalue");
  params.addParam<bool>("output_pi_history", false, "True to output solutions durint PI");
  return params;
}

NonlinearEigen::NonlinearEigen(const std::string & name, InputParameters parameters)
    :EigenExecutionerBase(name, parameters),
     // local static memebers
     _free_iter(getParam<unsigned int>("free_power_iterations")),
     _abs_tol(getParam<Real>("source_abs_tol")),
     _rel_tol(getParam<Real>("source_rel_tol")),
     _pfactor(getParam<Real>("pfactor")),
     _output_pi(getParam<bool>("output_pi_history"))
{
  _eigenvalue = getParam<Real>("k0");
  addRealParameterReporter("eigenvalue");

  if (getParam<bool>("output_on_final") && _output_pi)
  {
    mooseWarning("Only final solution will be outputted, output_pi_history=true will be ignored!");
    _output_pi = false;
  }
}

void
NonlinearEigen::execute()
{
  preExecute();

  // save the initial guess
  _problem.copyOldSolutions();

  Real initial_res = 0.0;
  if (_free_iter>0)
  {
    // free power iterations
    Moose::out << std::endl << " Free power iteration starts"  << std::endl;

    inversePowerIteration(_free_iter, _free_iter, _pfactor, false,
                          std::numeric_limits<Real>::min(), std::numeric_limits<Real>::max(),
                          true, _output_pi, 0.0,
                          _eigenvalue, initial_res);

    _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::PRE_AUX);
    _problem.onTimestepEnd();
    _problem.computeAuxiliaryKernels(EXEC_TIMESTEP);
    _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::POST_AUX);
  }

  if (!getParam<bool>("output_on_final"))
  {
    _problem.timeStep() = POWERITERATION_END;
    Real t = _problem.time();
    _problem.time() = _problem.timeStep();
    _output_warehouse.outputStep();
    _problem.time() = t;
  }

  Moose::out << " Nonlinear iteration starts"  << std::endl;

  _problem.timestepSetup();
  _problem.copyOldSolutions();

  Real rel_tol = _rel_tol;
  Real abs_tol = _abs_tol;
  if (_free_iter>0)
  {
    Moose::out << " Initial |R| = " << initial_res << std::endl;
    abs_tol = _rel_tol*initial_res;
    if (abs_tol<_abs_tol) abs_tol = _abs_tol;
    rel_tol = 1e-50;
  }

  // nonlinear solve
  preSolve();
  nonlinearSolve(rel_tol, abs_tol, _pfactor, _eigenvalue);
  postSolve();

  _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::PRE_AUX);
  _problem.onTimestepEnd();
  _problem.computeAuxiliaryKernels(EXEC_TIMESTEP);
  _problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::POST_AUX);
  if (_run_custom_uo)
  {
    _problem.computeUserObjects(EXEC_CUSTOM);
    _problem.computeAuxiliaryKernels(EXEC_CUSTOM);
    _problem.computeUserObjects(EXEC_CUSTOM, UserObjectWarehouse::POST_AUX);
  }

  if (!getParam<bool>("output_on_final"))
  {
    _problem.timeStep() = NONLINEAR_SOLVE_END;
    Real t = _problem.time();
    _problem.time() = _problem.timeStep();
    _output_warehouse.outputStep();
    _problem.time() = t;
  }

  Real s = normalizeSolution(_norm_execflag!=EXEC_CUSTOM && _norm_execflag!=EXEC_TIMESTEP &&
                             _norm_execflag!=EXEC_RESIDUAL);

  Moose::out << " Solution is rescaled with factor " << s << " for normalization!" << std::endl;

  if (getParam<bool>("output_on_final") || std::fabs(s-1.0)>std::numeric_limits<Real>::epsilon())
  {
    _problem.timeStep() = FINAL;
    Real t = _problem.time();
    _problem.time() = _problem.timeStep();
    _output_warehouse.outputStep();
    _problem.time() = t;
  }

  postExecute();
}

void
NonlinearEigen::postSolve()
{
  printEigenvalue();
}
