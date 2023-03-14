//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Predictor.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "Transient.h"

#include "libmesh/numeric_vector.h"

InputParameters
Predictor::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addRequiredParam<Real>("scale",
                                "The scale factor for the predictor (can range from 0 to 1)");
  params.addParam<std::vector<Real>>(
      "skip_times", "Skip the predictor if the current solution time is in this list of times");
  params.addParam<std::vector<Real>>(
      "skip_times_old",
      "Skip the predictor if the previous solution time is in this list of times");
  params.addParam<bool>("skip_after_failed_timestep",
                        false,
                        "Skip prediction in a repeated time step after a failed time step");

  params.registerBase("Predictor");

  return params;
}

Predictor::Predictor(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(this, "Predictors"),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _nl(_fe_problem.getNonlinearSystemBase()),
    _t_step(_fe_problem.timeStep()),
    _dt(_fe_problem.dt()),
    _dt_old(_fe_problem.dtOld()),
    _solution(*_nl.currentSolution()),
    _solution_old(_nl.solutionOld()),
    _solution_older(_nl.solutionOlder()),
    _solution_predictor(_nl.addVector("predictor", true, GHOSTED)),
    _t_step_old(declareRestartableData<int>("t_step_old", 0)),
    _is_repeated_timestep(declareRestartableData<bool>("is_repeated_timestep", false)),
    _scale(getParam<Real>("scale")),
    _skip_times(getParam<std::vector<Real>>("skip_times")),
    _skip_times_old(getParam<std::vector<Real>>("skip_times_old")),
    _skip_after_failed_timetep(getParam<bool>("skip_after_failed_timestep")),
    _timestep_tolerance(dynamic_cast<Transient *>(_app.getExecutioner())->timestepTol())
{
  if (_scale < 0.0 || _scale > 1.0)
    mooseError("Input value for scale = ", _scale, " is outside of permissible range (0 to 1)");
}

Predictor::~Predictor() {}

void
Predictor::timestepSetup()
{
  _is_repeated_timestep = false;

  // if the time step number hasn't changed
  // we are recomputing a failed time step
  if (_t_step == _t_step_old)
    _is_repeated_timestep = true;

  _t_step_old = _t_step;
}

bool
Predictor::shouldApply()
{
  bool should_apply = true;

  // if no prediction in a repeated timestep should be made
  if (_is_repeated_timestep && _skip_after_failed_timetep)
    should_apply = false;

  const Real & current_time = _fe_problem.time();
  const Real & old_time = _fe_problem.timeOld();
  for (unsigned int i = 0; i < _skip_times.size() && should_apply; ++i)
  {
    if (MooseUtils::absoluteFuzzyEqual(current_time, _skip_times[i], _timestep_tolerance))
      should_apply = false;
  }
  for (unsigned int i = 0; i < _skip_times_old.size() && should_apply; ++i)
  {
    if (MooseUtils::absoluteFuzzyEqual(old_time, _skip_times_old[i], _timestep_tolerance))
      should_apply = false;
  }
  return should_apply;
}
