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

// MOOSE includes
#include "DT2.h"
#include "FEProblem.h"
#include "TimeIntegrator.h"
#include "NonlinearSystemBase.h"
#include "Stepper.h"
#include "Transient.h"

//libMesh includes
#include "libmesh/implicit_system.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/transient_system.h"
#include "libmesh/numeric_vector.h"

// C++ Includes
#include <iomanip>


template<>
InputParameters validParams<DT2>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addParam<Real>("dt", 1., "The initial time step size.");
  params.addRequiredParam<Real>("e_tol", "Target error tolerance.");
  params.addRequiredParam<Real>("e_max", "Maximum acceptable error.");
  params.addParam<Real>("max_increase", 1.0e9, "Maximum ratio that the time step can increase.");

  return params;
}

DT2::DT2(const InputParameters & parameters) :
    TimeStepper(parameters),
    _e_tol(getParam<Real>("e_tol")),
    _e_max(getParam<Real>("e_max")),
    _max_increase(getParam<Real>("max_increase"))
{}

StepperBlock *
DT2::buildStepper()
{
  StepperBlock * s = new DT2Block(_executioner.timestepTol(), _e_tol, _e_max, _fe_problem.getNonlinearSystem().getTimeIntegrator()->order());
  s = BaseStepper::maxRatio(s, _max_increase);
  s = BaseStepper::initialN(BaseStepper::constant(getParam<Real>("dt")), s, _executioner.n_startup_steps());
  return s;
}
