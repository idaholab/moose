//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExplicitMixedOrderStasis.h"
#include "PostprocessorInterface.h"

registerMooseObject("SolidMechanicsApp", ExplicitMixedOrderStasis);

InputParameters
ExplicitMixedOrderStasis::validParams()
{
  InputParameters params = ExplicitMixedOrder::validParams();

  params.addClassDescription(
      "ExplicitMixedOrder time integrator, with the additional ability to perform time steps in "
      "which velocity and acceleration are set to zero, and the primary variables are not changed");

  params.addRequiredParam<PostprocessorName>(
      "in_stasis",
      "Postprocessor that defines whether the time stepping is in stasis or not.  If in_stasis==1 "
      "then time stepping involves setting v=0, and a=0 for second-order variables, and not "
      "changing the primary variables.  Otherwise, ordinary time stepping is used, as per the "
      "ExplicitMixedOrder time stepper");
  return params;
}

ExplicitMixedOrderStasis::ExplicitMixedOrderStasis(const InputParameters & parameters)
  : ExplicitMixedOrder(parameters),
    PostprocessorInterface(this),
    _in_stasis(getPostprocessorValue("in_stasis")),
    _prev_timestep_in_stasis(false),
    _dt_old_to_use(0.0)
{
}

void
ExplicitMixedOrderStasis::init()
{
  _prev_timestep_in_stasis = (_in_stasis == 1);
  _dt_old_to_use = _dt_old;
  ExplicitMixedOrder::init();
}

void
ExplicitMixedOrderStasis::solve()
{
  /*
  If the previous time step is not in stasis then record its dt.
  Now:
  - If this current time step is in stasis, then _dt_old_to_use is not used,
    because performExplicitSolve simply sets v = 0 = a
  - If this current time step is not in stasis, then:
     - if the previous time step was not in stasis then setting
       _dt_old_to_use = _dt_old means the central differening just uses
       (_dt_old + _dt) / 2.  This is exactly the usual ExplicitMixedOrder
       case.
     - if the previous time step was in stasis, then the following will
       NOT set _dt_old_to_use: instead, _dt_old_to_use must have been
       set prior to the current stasis period, so the central differencing
       will use _dt_old_to_use = _dt before the current stasis period
  */
  if (!_prev_timestep_in_stasis)
    _dt_old_to_use = _dt_old;

  ExplicitMixedOrder::solve();
}

void
ExplicitMixedOrderStasis::postResidual(NumericVector<Number> & residual)
{
  // set _prev_timestep_in_stasis correctly for next time step
  _prev_timestep_in_stasis = (_in_stasis == 1);

  ExplicitMixedOrder::postResidual(residual);
}

bool
ExplicitMixedOrderStasis::performExplicitSolve(SparseMatrix<Number> & mass_matrix)
{
  if (_in_stasis == 1)
  {
    auto & accel = *_sys.solutionUDotDot();
    accel.zero();
    accel.close();

    auto & vel = *_sys.solutionUDot();
    vel.zero();
    vel.close();

    //*_solution_update->zero();
    _solution_update->zero();

    _n_linear_iterations = 0;
    return true;
  }
  else
    return ExplicitMixedOrder::performExplicitSolve(mass_matrix);
}

void
ExplicitMixedOrderStasis::computeICs()
{
  if (_in_stasis == 1)
  {
    auto & vel = *_sys.solutionUDot();
    vel.zero();
    vel.close();
    // Since this is an IC, there is no guaranteed correct _dt_old
    // The following choice seems to be the most reasonable.
    _dt_old_to_use = _dt;
  }
  else
    ExplicitMixedOrder::computeICs();
}

Real
ExplicitMixedOrderStasis::centralDifferenceDt()
{
  return 0.5 * (_dt + _dt_old_to_use);
}
