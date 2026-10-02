//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PIDAdaptiveDT.h"

#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "MooseVariableFieldBase.h"
#include "TransientBase.h"

#include "libmesh/system.h"
#include "libmesh/enum_norm_type.h"

registerMooseObject("MooseApp", PIDAdaptiveDT);

InputParameters
PIDAdaptiveDT::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addClassDescription(
      "Computes time step size with a PID control logic on estimated local time-stepping error");
  params.addRequiredRangeCheckedParam<Real>(
      "dt_initial", "dt_initial > 0", "The initial time step size");
  MooseEnum opt("2nd_derivative=0 1st_derivative=1", "2nd_derivative");
  params.addParam<MooseEnum>(
      "error_esitmation_type", opt, "The option on how to estimate the local error");
  params.addRangeCheckedParam<Real>("local_error_tolerance",
                                    1e-3,
                                    "local_error_tolerance > 0",
                                    "The target estimated local error");
  params.addParam<std::vector<VariableName>>(
      "variables",
      "Variables used to estimate the local time-stepping error (default to use all variables in "
      "the nonlinear system)");
  params.addParam<Real>("proportional_gain", 0.05, "The proportional gain");
  params.addParam<Real>("integral_gain", 0.0, "The integral gain");
  params.addParam<Real>("derivative_gain", 0.0, "The derivative gain");
  params.addRangeCheckedParam<Real>("max_increase_factor",
                                    2,
                                    "max_increase_factor >= 1",
                                    "The maximum factor by which the current time step size will "
                                    "increase with respect to the previous time step size.");
  params.addRangeCheckedParam<Real>("min_decrease_factor",
                                    0.5,
                                    "min_decrease_factor <= 1",
                                    "The minimum factor by which the current time step size will "
                                    "decrease with respect to the previous time step size.");
  params.addParam<PostprocessorName>("timestep_limiting_postprocessor",
                                     std::numeric_limits<Real>::max(),
                                     "A postprocessor to limit the time step size.");

  return params;
}

PIDAdaptiveDT::PIDAdaptiveDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    PostprocessorInterface(this),
    _dt_initial(getParam<Real>("dt_initial")),
    _error_estimation_option(getParam<MooseEnum>("error_esitmation_type")),
    _auto_tolerance(!isParamSetByUser("local_error_tolerance")),
    _local_error_tolerance(getParam<Real>("local_error_tolerance")),
    _variables(isParamValid("variables")
                   ? getParam<std::vector<VariableName>>("variables")
                   : _fe_problem.getNonlinearSystemBase(0).getVariableNames()),
    _proportional_gain(getParam<Real>("proportional_gain")),
    _integral_gain(getParam<Real>("integral_gain")),
    _derivative_gain(getParam<Real>("derivative_gain")),
    _max_increase_factor(getParam<Real>("max_increase_factor")),
    _min_decrease_factor(getParam<Real>("min_decrease_factor")),
    _pp_dt_limiter(getPostprocessorValue("timestep_limiting_postprocessor")),
    _dt_old(declareRestartableData<Real>("dt_old", 0.0)),
    _error(declareRestartableData<Real>("local_error", 1.0)),
    _error_old(declareRestartableData<Real>("local_error_old", 1.0)),
    _error_older(declareRestartableData<Real>("local_error_older", 1.0)),
    _initial_error(
        declareRestartableData<Real>("initial_local_error", std::numeric_limits<Real>::max()))
{
  // get variable systems and numbers
  for (const auto & var_name : _variables)
  {
    auto & var = _fe_problem.getActualFieldVariable(0, var_name);

    auto & sys = var.sys();
    std::vector<unsigned int> all_variable_numbers;
    sys.system().get_all_variable_numbers(all_variable_numbers);
    for (const auto & vnumber : all_variable_numbers)
    {
      _calc_norm[&var.sys()].set_weight(vnumber, 0);
      _calc_norm[&var.sys()].set_type(vnumber, libMesh::L2);
    }

    const auto & factor = var.arrayScalingFactor();
    for (const auto i : make_range(var.count()))
      _calc_norm[&var.sys()].set_weight(var.number() + i, factor[i]);

    // request older solution when using the second-order formulation to evaluate the local error
    if (_error_estimation_option == 0)
      var.sys().solutionState(2);
  }
}

Real
PIDAdaptiveDT::computeInitialDT()
{
  return _dt_initial;
}

Real
PIDAdaptiveDT::computeDT()
{
  // apply PID to get the DT
  Real dt = std::pow(_error, -_proportional_gain) *
            std::pow(_error_old, _derivative_gain - _integral_gain) *
            std::pow(_error_older, -_derivative_gain) * _dt_old;

  if (std::isinf(dt))
    // this can happen when error is zero and exponent in the above pow function calls is negative
    // we will use the initial time step size
    dt = _dt_initial;

  if (_executioner.verbose())
  {
    const Real e = _auto_tolerance ? _error * _initial_error : _error * _local_error_tolerance;
    mooseInfoRepeated("Error=", e, " DT_old=", _dt_old, " DT=", dt, " DT_ratio=", dt / _dt_old);
  }
  return dt;
}

bool
PIDAdaptiveDT::constrainStep(Real & dt)
{
  // apply various limiters
  if (_dt_old != 0.0)
  {
    if (dt > _dt_old * _max_increase_factor)
      dt = _dt_old * _max_increase_factor;
    if (dt < _dt_old * _min_decrease_factor)
      dt = _dt_old * _min_decrease_factor;
  }
  if (dt > _pp_dt_limiter)
    dt = _pp_dt_limiter;

  return TimeStepper::constrainStep(dt);
}

void
PIDAdaptiveDT::acceptStep()
{
  TimeStepper::acceptStep();
  _dt_old = _dt;
  _error_old = _error;
  _error_older = _error_old;

  // evaluate the local error
  // Note: this evaluation cannot be in computeDT() because the system just advanced the state
  //       and the current solution is not available.
  const bool first_eval = _initial_error == std::numeric_limits<Real>::max();
  Real var_l2_norm = 0;
  Real var_error_norm = 0;
  for (const auto & [sys, calc_norm] : _calc_norm)
  {
    Real v = sys->system().calculate_norm(sys->solution(), calc_norm);
    var_l2_norm += v * v;

    auto vec = sys->solution().clone();
    if (_error_estimation_option == 0)
    {
      mooseAssert(_dt_old != 0, "Old DT should have been initialized at this point");
      Real dt_ratio = _dt / _dt_old;
      vec->add(-1 - dt_ratio, sys->solutionOld());
      if (first_eval)
      {
        mooseAssert(sys->solutionOlder().l2_norm() == 0,
                    "Older solution has not been available yet");
        vec->add(dt_ratio, sys->solutionOld());
      }
      else
        vec->add(dt_ratio, sys->solutionOlder());
      vec->scale(0.5);
    }
    else if (_error_estimation_option == 1)
      vec->add(-1, sys->solutionOld());
    else
      mooseError("Unsupported option of local error evaluation");

    for (const auto & [sys, calc_norm] : _calc_norm)
    {
      Real v = sys->system().calculate_norm(*vec, calc_norm);
      var_error_norm += v * v;
    }
  }
  var_l2_norm = std::sqrt(var_l2_norm);
  if (var_l2_norm == 0.0)
  {
    _error = 0.0;
    return;
  }
  var_error_norm = std::sqrt(var_error_norm);

  if (first_eval && var_error_norm != 0.0)
  {
    _initial_error = var_error_norm / var_l2_norm;
    if (_auto_tolerance && _executioner.verbose())
      mooseInfoRepeated("Estimated initial error = ", _initial_error);
  }

  if (_auto_tolerance)
    // we attempt to not change the initial DT
    _error = var_error_norm / var_l2_norm / _initial_error;
  else
    _error = var_error_norm / var_l2_norm / _local_error_tolerance;
}
