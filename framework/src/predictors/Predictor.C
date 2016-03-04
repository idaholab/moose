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
#include "Predictor.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"

// libMesh includes
#include "libmesh/numeric_vector.h"

template<>
InputParameters validParams<Predictor>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<Real>("scale", "The scale factor for the predictor (can range from 0 to 1)");
  params.addParam<std::vector<Real> >("skip_times", "Time steps for which the predictor should not be applied");

  params.registerBase("Predictor");

  return params;
}

Predictor::Predictor(const InputParameters & parameters) :
    MooseObject(parameters),
    Restartable(parameters, "Predictors"),
    _fe_problem(*parameters.getCheckedPointerParam<FEProblem *>("_fe_problem")),
    _nl(_fe_problem.getNonlinearSystem()),

    _t_step(_fe_problem.timeStep()),
    _dt(_fe_problem.dt()),
    _dt_old(_fe_problem.dtOld()),
    _solution(*_nl.currentSolution()),
    _solution_old(_nl.solutionOld()),
    _solution_older(_nl.solutionOlder()),
    _solution_predictor(_nl.addVector("predictor", true, GHOSTED)),
    _scale(getParam<Real>("scale")),
    _skip_times(getParam<std::vector<Real> >("skip_times"))
{
  if (_scale < 0.0 || _scale > 1.0)
    mooseError("Input value for scale = " << _scale << " is outside of permissible range (0 to 1)");
}

Predictor::~Predictor()
{
}

void
Predictor::timestepSetup()
{
}

bool
Predictor::shouldApply()
{
  bool should_apply = true;

  const Real & current_time =  _fe_problem.time();
  for (unsigned int i=0; i<_skip_times.size(); ++i)
  {
    if (MooseUtils::absoluteFuzzyEqual(current_time, _skip_times[i]))
    {
      should_apply = false;
      break;
    }
  }
  return should_apply;
}
