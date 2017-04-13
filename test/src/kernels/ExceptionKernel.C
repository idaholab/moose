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
  return params;
}

ExceptionKernel::ExceptionKernel(const InputParameters & parameters)
  : Kernel(parameters), _when(static_cast<WhenType>((int)getParam<MooseEnum>("when")))
{
}

Real
ExceptionKernel::computeQpResidual()
{
  // We need a thread lock here so that we don't introduce a race condition when inspecting or
  // changing the static variable
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mutex);

  if (_when == INITIAL_CONDITION)
    throw MooseException("MooseException thrown during initial condition computation");

  // Make sure we have called computeQpResidual enough times to
  // guarantee that we are in the middle of a linear solve, to verify
  // that we can throw an exception at that point.
  else if (_when == RESIDUAL && !_res_has_thrown && time_to_throw())
  {
    // The residual has now thrown
    _res_has_thrown = true;

    throw MooseException("MooseException thrown during residual calculation");
  }
  else
    return 0;
}

Real
ExceptionKernel::computeQpJacobian()
{
  // We need a thread lock here so that we don't introduce a race condition when inspecting or
  // changing the static variable
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mutex);

  // Throw on the first nonlinear step of the first timestep -- should
  // hopefully be the same in both serial and parallel.
  if (_when == JACOBIAN && !_jac_has_thrown && time_to_throw())
  {
    // The Jacobian has now thrown
    _jac_has_thrown = true;

    throw MooseException("MooseException thrown during Jacobian calculation");
  }

  return 0.;
}

bool
ExceptionKernel::time_to_throw()
{
  return (_t_step == 1 &&
          _fe_problem.getNonlinearSystemBase().getCurrentNonlinearIterationNumber() == 1);
}
