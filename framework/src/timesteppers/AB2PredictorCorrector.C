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

#include "AB2PredictorCorrector.h"
#include "AdamsPredictor.h"
#include "Problem.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"
#include "TimeIntegrator.h"
#include "Stepper.h"

//libMesh includes
#include "libmesh/nonlinear_solver.h"
#include "libmesh/numeric_vector.h"

// C++ Includes
#include <iomanip>
#include <iostream>
#include <fstream>

template<>
InputParameters validParams<AB2PredictorCorrector>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addRequiredParam<Real>("e_tol","Target error tolerance.");
  params.addRequiredParam<Real>("e_max","Maximum acceptable error.");
  params.addRequiredParam<Real>("dt", "Initial time step size");
  params.addParam<Real>("max_increase", 1.0e9,    "Maximum ratio that the time step can increase.");
  params.addParam<int>("steps_between_increase",1,"the number of time steps before recalculating dt");
  params.addParam<int>("start_adapting",2, "when to start taking adaptive time steps");
  params.addParam<Real>("scaling_parameter", .8, "scaling parameter for dt selection");
  return params;
}

AB2PredictorCorrector::AB2PredictorCorrector(const InputParameters & parameters) :
    TimeStepper(parameters),
    _e_tol(getParam<Real>("e_tol")),
    _e_max(getParam<Real>("e_max")),
    _max_increase(getParam<Real>("max_increase")),
    _steps_between_increase(getParam<int>("steps_between_increase")),
    _start_adapting(getParam<int>("start_adapting")),
    _scaling_parameter(getParam<Real>("scaling_parameter"))
{
  Real predscale = 1.;
  InputParameters params = _app.getFactory().getValidParams("AdamsPredictor");
  params.set<Real>("scale") = predscale;
  _fe_problem.addPredictor("AdamsPredictor", "adamspredictor", params);

}

Stepper *
AB2PredictorCorrector::buildStepper()
{
  std::string integrator = _fe_problem.getNonlinearSystem().getTimeIntegrator()->name();

  Stepper * s = new PredictorCorrectorStepper(_start_adapting, _e_tol, _scaling_parameter, integrator);
  s = new MaxRatioStepper(s, _max_increase);
  s = new EveryNStepper(s, _steps_between_increase, _start_adapting);
  s = new StartupStepper(s, getParam<Real>("dt"), _start_adapting);
  s = new IfConvergedStepper(s, new GrowShrinkStepper(0.5, 1.0));
  return s;
}

