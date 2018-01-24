//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExceptionKernel.h"
#include "MooseException.h"
#include "NonlinearSystem.h"

#include "libmesh/threads.h"

/**
 * Class static initialization:
 * These members are used to create easily accessed shared memory among threads
 * without polluting framework classes.
 *
 * Note: Due to the use of statics, multiple ExceptionKernels will not
 * perform independently if used in the same simulation. That case isn't
 * intended to work anyway.
 */
bool ExceptionKernel::_res_has_thrown = false;
bool ExceptionKernel::_jac_has_thrown = false;

template <>
InputParameters
validParams<ExceptionKernel>()
{
  InputParameters params = validParams<Kernel>();
  MooseEnum when("residual=0 jacobian initial_condition", "residual");
  params.addParam<MooseEnum>("when", when, "When to throw the exception");
  params.addParam<bool>(
      "should_throw", true, "Toggle between throwing an exception or triggering an error");
  params.addParam<processor_id_type>(
      "rank", DofObject::invalid_processor_id, "Isolate an exception to a particular rank");
  return params;
}

ExceptionKernel::ExceptionKernel(const InputParameters & parameters)
  : Kernel(parameters),
    _when(static_cast<WhenType>((int)getParam<MooseEnum>("when"))),
    _should_throw(getParam<bool>("should_throw")),
    _rank(getParam<processor_id_type>("rank"))
{
}

Real
ExceptionKernel::computeQpResidual()
{
  // We need a thread lock here so that we don't introduce a race condition when inspecting or
  // changing the static variable
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mutex);

  if (_when == WhenType::INITIAL_CONDITION)
    throw MooseException("MooseException thrown during initial condition computation");

  // Make sure we have called computeQpResidual enough times to
  // guarantee that we are in the middle of a linear solve, to verify
  // that we can throw an exception at that point.
  // Also, only throw on one rank if the user requests it
  else if (_when == WhenType::RESIDUAL && !_res_has_thrown && time_to_throw() &&
           (_rank == DofObject::invalid_processor_id || _rank == processor_id()))
  {
    // The residual has now thrown
    _res_has_thrown = true;

    if (_should_throw)
      throw MooseException("MooseException thrown during residual calculation");
    else
      mooseError("Intentional error triggered during residual calculation");
  }
  else
    return 0.;
}

Real
ExceptionKernel::computeQpJacobian()
{
  // We need a thread lock here so that we don't introduce a race condition when inspecting or
  // changing the static variable
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mutex);

  // Throw on the first nonlinear step of the first timestep -- should
  // hopefully be the same in both serial and parallel.
  if (_when == WhenType::JACOBIAN && !_jac_has_thrown && time_to_throw() &&
      (_rank == DofObject::invalid_processor_id || _rank == processor_id()))
  {
    // The Jacobian has now thrown
    _jac_has_thrown = true;

    if (_should_throw)
      throw MooseException("MooseException thrown during Jacobian calculation");
    else
      mooseError("Intentional error triggered during Jacobian calculation");
  }

  return 0.;
}

bool
ExceptionKernel::time_to_throw() const
{
  return (_t_step == 1 &&
          _fe_problem.getNonlinearSystemBase().getCurrentNonlinearIterationNumber() == 1);
}
