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

#include "libmesh/numeric_vector.h"

template <>
InputParameters
validParams<Predictor>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<Real>("scale",
                                "The scale factor for the predictor (can range from 0 to 1)");
  params.addParam<std::vector<Real>>(
      "skip_times", "Skip the predictor if the current solution time is in this list of times");
  params.addParam<std::vector<Real>>(
      "skip_times_old",
      "Skip the predictor if the previous solution time is in this list of times");

  params.registerBase("Predictor");

  return params;
}

Predictor::Predictor(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(parameters, "Predictors"),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _nl(_fe_problem.getNonlinearSystemBase()),

    _t_step(_fe_problem.timeStep()),
    _dt(_fe_problem.dt()),
    _dt_old(_fe_problem.dtOld()),
    _solution(*_nl.currentSolution()),
    _solution_old(_nl.solutionOld()),
    _solution_older(_nl.solutionOlder()),
    _solution_predictor(_nl.addVector("predictor", true, GHOSTED)),
    _scale(getParam<Real>("scale")),
    _skip_times(getParam<std::vector<Real>>("skip_times")),
    _skip_times_old(getParam<std::vector<Real>>("skip_times_old"))
{
  if (_scale < 0.0 || _scale > 1.0)
    mooseError("Input value for scale = ", _scale, " is outside of permissible range (0 to 1)");
}

Predictor::~Predictor() {}

void
Predictor::timestepSetup()
{
}

bool
Predictor::shouldApply()
{
  bool should_apply = true;

  const Real & current_time = _fe_problem.time();
  const Real & old_time = _fe_problem.timeOld();
  for (unsigned int i = 0; i < _skip_times.size() && should_apply; ++i)
  {
    if (MooseUtils::absoluteFuzzyEqual(current_time, _skip_times[i]))
      should_apply = false;
  }
  for (unsigned int i = 0; i < _skip_times_old.size() && should_apply; ++i)
  {
    if (MooseUtils::absoluteFuzzyEqual(old_time, _skip_times_old[i]))
      should_apply = false;
  }
  return should_apply;
}
