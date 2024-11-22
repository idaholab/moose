//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "TransientAndAdjoint.h"
#include "OptimizationAppTypes.h"

registerMooseObject("OptimizationApp", TransientAndAdjoint);

InputParameters
TransientAndAdjoint::validParams()
{
  InputParameters params = Transient::validParams();
  params += AdjointTransientSolve::validParams();
  params.addClassDescription("Executioner for evaluating transient simulations and their adjoint.");

  // We need to full matrix for the adjoint solve, so set this to NEWTON
  params.set<MooseEnum>("solve_type") = "newton";
  params.suppressParameter<MooseEnum>("solve_type");

  // The adjoint system (second one) is solved by _adjoint_solve
  // This is a parameter of the MultiSystemSolveObject, which we set from here, the executioner.
  // We seek to prevent the MultiSystemSolveObject from solving both systems
  // This is abusing input parameters, but SolveObjects do not have their own syntax
  // and we need to send this parameter from the executioner to the default nested SolveObject
  params.renameParam("system_names", "forward_system", "");

  return params;
}

TransientAndAdjoint::TransientAndAdjoint(const InputParameters & parameters)
  : Transient(parameters),
    _adjoint_solve(*this),
    _forward_times(declareRecoverableData<std::vector<Real>>("forward_times"))
{
  // Error out on unsupported time integration schemes
  switch (getTimeScheme())
  {
    case Moose::TI_IMPLICIT_EULER:
      break;
    default:
      paramError(
          "scheme", getParam<MooseEnum>("scheme"), " is not supported for computing adjoint.");
      break;
  }
}

void
TransientAndAdjoint::preExecute()
{
  Transient::preExecute();

  // Save the forward initial condition
  _adjoint_solve.insertForwardSolution(_t_step);

  // Save the time of the initial condition
  // The vector of times should be reset unless we are recovering
  if (!_app.isRecovering())
    _forward_times = {_time};
  else
    _forward_times.push_back(_time);
}

void
TransientAndAdjoint::postStep()
{
  Transient::postStep();

  // Save the converged forward solution and time
  if (lastSolveConverged())
  {
    _adjoint_solve.insertForwardSolution(_t_step);
    _forward_times.push_back(_time);
  }
}

void
TransientAndAdjoint::postExecute()
{
  Transient::postExecute();

  // If it is a half transient, then the app is meant to be run with recovery. Therefore, it doesn't
  // make sense to run the adjoint calculation since we aren't getting to the final time required
  // for a consistent adjoint.
  if (_app.testCheckpointHalfTransient())
    return;

  // Looping backward through forward time steps
  for (const auto n : make_range(_forward_times.size() - 1))
  {
    // Set important time information so the Jacobian is properly computed in the forward system
    _t_step = _forward_times.size() - 1 - n;
    _time = _forward_times[_t_step];
    _time_old = _forward_times[_t_step - 1];
    _dt = _time - _time_old;
    _dt_old = _t_step < 2 ? _dt : _time_old - _forward_times[_t_step - 2];

    // Set the forward solution to the time step that is currently being solved
    _adjoint_solve.setForwardSolution(_t_step);

    // Incase there are some residual objects that need this call
    _problem.timestepSetup();
    _problem.onTimestepBegin();

    // Solve the adjoint system
    _last_solve_converged = _adjoint_solve.solve();
    if (!lastSolveConverged())
      break;

    // FIXME: This works well enough for console and CSV output, but exodus is a mess since I don't
    // think it knows what to do with backward timestepping or duplicate outputs at a given time
    _problem.outputStep(OptimizationAppTypes::EXEC_ADJOINT_TIMESTEP_END);
  }
}
