//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppNewtonIterationUserObject.h"

#include "Executioner.h"
#include "FEProblemBase.h"
#include "Factory.h"
#include "Function.h"
#include "MultiApp.h"

registerMooseObject("MooseApp", MultiAppNewtonIterationUserObject);

InputParameters
MultiAppNewtonIterationUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Drives a TransientMultiApp through a Newton iteration to find the parameter value "
      "that yields a target output (given as a function of time).");

  params.addRequiredParam<FileName>(
      "sub_app_input", "Input file for the internally-created TransientMultiApp instance.");

  params.addRequiredParam<PostprocessorName>(
      "param_postprocessor",
      "Name of the Receiver postprocessor in the sub-app that receives the parameter value.");
  params.addRequiredParam<PostprocessorName>(
      "output_postprocessor",
      "Name of the postprocessor in the sub-app whose value is compared against the target.");

  params.addRequiredParam<FunctionName>(
      "target_function", "Function f(t) specifying the target output value at each time step.");

  params.addRequiredParam<Real>("initial_parameter",
                                "Initial guess for the parameter (used for the first time step).");

  params.addRangeCheckedParam<Real>("delta_parameter",
                                    1e-4,
                                    "delta_parameter>0",
                                    "Perturbation used to estimate df/dp by finite difference.");
  params.addRangeCheckedParam<Real>(
      "abs_tol", 1e-8, "abs_tol>0", "Absolute convergence tolerance on |output - target|.");
  params.addRangeCheckedParam<Real>(
      "rel_tol",
      1e-6,
      "rel_tol>0",
      "Relative convergence tolerance on |output - target| / |target|.");
  params.addRangeCheckedParam<unsigned int>("max_iterations",
                                            50,
                                            "max_iterations>0",
                                            "Maximum number of Newton iterations per time step.");
  params.addParam<bool>(
      "accept_on_max_iteration",
      false,
      "If true, accept the best estimate (with a warning) when max_iterations is reached without "
      "convergence. If false (default), cut the time step and retry, erroring if the time step "
      "cannot be reduced further.");

  params.addParam<PostprocessorName>(
      "parameter_postprocessor",
      "Optional name of a Receiver postprocessor in the main app to which the "
      "converged parameter value is written after each time step.");

  params.addParam<bool>(
      "concurrent_perturbation",
      false,
      "If true, evaluate the p and p+delta_parameter solves concurrently on two sub-apps "
      "(faster on >= 2 MPI ranks, ~2x memory) instead of sequentially on one sub-app.");

  // Run at TIMESTEP_BEGIN so the converged sub-app state is available to FROM_MULTIAPP transfers.
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;

  return params;
}

MultiAppNewtonIterationUserObject::MultiAppNewtonIterationUserObject(
    const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _multiapp_name(name() + "_app"),
    _param_pp(getParam<PostprocessorName>("param_postprocessor")),
    _output_pp(getParam<PostprocessorName>("output_postprocessor")),
    _target_function(getFunction("target_function")),
    _delta_param(getParam<Real>("delta_parameter")),
    _abs_tol(getParam<Real>("abs_tol")),
    _rel_tol(getParam<Real>("rel_tol")),
    _max_its(getParam<unsigned int>("max_iterations")),
    _param_value(declareRestartableData<Real>("param_value", getParam<Real>("initial_parameter"))),
    _param_value_step_begin(declareRestartableData<Real>("param_value_step_begin",
                                                         getParam<Real>("initial_parameter"))),
    _executed_t_step(declareRestartableData<int>("executed_t_step", -1)),
    _param_output_pp(isParamValid("parameter_postprocessor")
                         ? getParam<PostprocessorName>("parameter_postprocessor")
                         : PostprocessorName("")),
    _concurrent(getParam<bool>("concurrent_perturbation")),
    _accept_on_max_iteration(getParam<bool>("accept_on_max_iteration"))
{
  // Internally create the TransientMultiApp (execute_on = NONE so only this UserObject drives
  // it) rather than requiring a [MultiApps] block. Concurrent mode uses two sub-apps so MOOSE
  // can solve p and p+delta simultaneously across ranks; sequential mode uses one.
  InputParameters mp = _app.getFactory().getValidParams("TransientMultiApp");
  mp.set<std::vector<FileName>>("input_files") = {getParam<FileName>("sub_app_input")};
  mp.set<ExecFlagEnum>("execute_on") = EXEC_NONE;
  if (_concurrent)
    mp.set<std::vector<Point>>("positions") = {libMesh::Point(), libMesh::Point()};
  _fe_problem.addMultiApp("TransientMultiApp", _multiapp_name, mp);

  if (_concurrent && _communicator.size() < 2)
    mooseInfo(name(),
              ": 'concurrent_perturbation' is enabled but the run uses a single MPI process, "
              "so the two sub-app solves still execute sequentially (with roughly twice the "
              "memory). Run with >= 2 MPI processes to benefit from concurrency.");
}

void
MultiAppNewtonIterationUserObject::execute()
{
  auto multiapp = _fe_problem.getMultiApp(_multiapp_name);

  const Real y_target = _target_function.value(_t, libMesh::Point());

  if (_t_step == _executed_t_step)
  {
    // This time step is being repeated (e.g. --test-restep, or a parent time-step cut).
    multiapp->restore();
    _param_value = _param_value_step_begin;
  }
  else
  {
    // First solve of this time step: back up the sub-app state so every Newton trial starts from
    // the same initial condition, and remember the starting parameter guess and step index so the
    // step can be faithfully repeated if it is later rejected.
    multiapp->backup();
    _param_value_step_begin = _param_value;
    _executed_t_step = _t_step;
  }

  Real p = _param_value;
  bool converged = false;

  for (unsigned int iter = 0; iter < _max_its; ++iter)
  {
    Real y1 = 0.0, y2 = 0.0;
    if (!solveTrial(multiapp, p, y1, y2))
    {
      mooseInfoRepeated(name(),
                        ": '",
                        _multiapp_name,
                        "' failed to converge during Newton iteration ",
                        iter + 1,
                        " at t = ",
                        _t,
                        ". Cutting timestep.");
      multiapp->restore();
      getMooseApp().getExecutioner()->fixedPointSolve().failStep();
      return;
    }

    const Real residual = y1 - y_target;

    // Check convergence on the base (p) solution.
    const Real tol = std::max(_abs_tol, _rel_tol * std::abs(y_target));
    if (std::abs(residual) <= tol)
    {
      converged = true;
      break;
    }

    // Finite-difference estimate of df/dp.
    const Real dy_dp = (y2 - y1) / _delta_param;

    if (std::abs(dy_dp) < 1e-15 * (std::abs(y1) + std::abs(y2) + 1.0))
    {
      mooseWarning(name(),
                   ": the estimated derivative df/dp = ",
                   dy_dp,
                   " is too small to perform a Newton update at t = ",
                   _t,
                   ". Cutting timestep.");
      multiapp->restore();
      getMooseApp().getExecutioner()->fixedPointSolve().failStep();
      return;
    }

    // Newton update.
    p -= residual / dy_dp;
  }

  if (!converged)
  {
    // Not converged within max_iterations: either cut the time step (default) or accept the
    // best estimate with a warning, per accept_on_max_iteration.
    if (!_accept_on_max_iteration)
    {
      // Use info (not a warning) so a '--error' run does not abort on the first cut; the run
      // ends with the executioner's error once the time step can no longer be reduced.
      mooseInfoRepeated(name(),
                        ": Newton iteration did not converge within ",
                        _max_its,
                        " iterations at t = ",
                        _t,
                        ". Cutting the time step.");
      multiapp->restore();
      getMooseApp().getExecutioner()->fixedPointSolve().failStep();
      return;
    }

    mooseWarning(name(),
                 ": Newton iteration did not converge within ",
                 _max_its,
                 " iterations at t = ",
                 _t,
                 ". Proceeding with the best estimate p = ",
                 p,
                 ".");
  }

  // Store the converged (or best) parameter for the next time step.
  _param_value = p;

  // Publish to the optional main-app Receiver postprocessor.
  if (!_param_output_pp.empty())
    _fe_problem.setPostprocessorValueByName(_param_output_pp, p);

  // Advance the sub-app(s) one time step at the converged p. The final solve at p is skipped
  // only when sequential AND converged, since solveTrial's last solve was already at p. In
  // concurrent mode sub-app 1 still holds p+delta and must be re-solved at p, because
  // finishStep() advances every local sub-app and a stale state would corrupt the next step's
  // backup. The solve uses auto_advance=false so finishStep() advances the clock exactly once
  // (auto_advance=true would double-advance it and cause the next solve to be skipped).
  if (!converged || _concurrent)
  {
    if (!finalSolveAtP(multiapp, p))
    {
      mooseWarning(name(),
                   ": '",
                   _multiapp_name,
                   "' failed during the final solve at t = ",
                   _t,
                   ". Cutting timestep.");
      multiapp->restore();
      getMooseApp().getExecutioner()->fixedPointSolve().failStep();
      return;
    }
  }

  // finishStep() must precede incrementTStep() so _time is updated to t_new before the step
  // counter captures _time_old.
  multiapp->finishStep();
  multiapp->incrementTStep(_t);
}

bool
MultiAppNewtonIterationUserObject::solveTrial(std::shared_ptr<MultiApp> & app,
                                              Real p,
                                              Real & y1,
                                              Real & y2) const
{
  if (_concurrent)
  {
    // Single solveStep evaluates sub-app 0 at p and sub-app 1 at p+delta; on >= 2 ranks these
    // run simultaneously on disjoint ranks.
    app->restore();
    setSubAppParam(app, 0, p);
    setSubAppParam(app, 1, p + _delta_param);
    const bool ok = app->solveStep(_dt, _t, /*auto_advance=*/false);
    y1 = ok ? getSubAppOutput(app, 0) : 0.0;
    y2 = ok ? getSubAppOutput(app, 1) : 0.0;
    return ok;
  }

  // Sequential: solve the single sub-app twice (restoring between). The base (p) solve is last
  // so on convergence the sub-app already holds the p-solution, avoiding an extra final solve.
  app->restore();
  setSubAppParam(app, 0, p + _delta_param);
  const bool ok_perturbed = app->solveStep(_dt, _t, /*auto_advance=*/false);
  y2 = ok_perturbed ? getSubAppOutput(app, 0) : 0.0;

  app->restore();
  setSubAppParam(app, 0, p);
  const bool ok_base = app->solveStep(_dt, _t, /*auto_advance=*/false);
  y1 = ok_base ? getSubAppOutput(app, 0) : 0.0;

  return ok_base && ok_perturbed;
}

bool
MultiAppNewtonIterationUserObject::finalSolveAtP(std::shared_ptr<MultiApp> & app, Real p) const
{
  app->restore();
  for (unsigned int i = 0; i < app->numGlobalApps(); ++i)
    setSubAppParam(app, i, p);
  return app->solveStep(_dt, _t, /*auto_advance=*/false);
}

void
MultiAppNewtonIterationUserObject::setSubAppParam(std::shared_ptr<MultiApp> & app,
                                                  unsigned int index,
                                                  Real value) const
{
  if (app->hasLocalApp(index))
    app->appProblemBase(index).setPostprocessorValueByName(_param_pp, value);
}

Real
MultiAppNewtonIterationUserObject::getSubAppOutput(std::shared_ptr<MultiApp> & app,
                                                   unsigned int index) const
{
  // The output postprocessor is a single scalar that MOOSE already replicates across the
  // sub-app's own communicator, so the sub-app's root rank holds the complete value. Read it
  // there only -- the hasLocalApp/isRootProcessor guard makes exactly one rank contribute --
  // and sum over the parent communicator to broadcast that one value to every rank. This is a
  // broadcast, not an aggregation: summing the value over all of a multi-rank sub-app's ranks
  // would over-count it, which the single-contributor guard is precisely there to avoid.
  Real value = 0.0;
  if (app->hasLocalApp(index) && app->isRootProcessor())
    value = app->appPostprocessorValue(index, _output_pp);

  _communicator.sum(value);
  return value;
}
