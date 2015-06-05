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

template<>
InputParameters validParams<ExceptionKernel>()
{
  InputParameters params = validParams<Kernel>();
  MooseEnum when("residual=0 jacobian initial_condition", "residual");
  params.addParam<MooseEnum>("when", when, "When to throw the exception");
  return params;
}


ExceptionKernel::ExceptionKernel(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _when(static_cast<WhenType>((int) getParam<MooseEnum>("when"))),
    _call_no(0),
    _jac_call_no(0)
{
}

Real
ExceptionKernel::computeQpResidual()
{
  // Increment the call number *before* we possibly throw an
  // exception, so this function can continue to be used after that.
  _call_no++;

  if (_when == INITIAL_CONDITION)
    throw MooseException("MooseException thrown during initial condition computation");

  // Make sure we have called computeQpResidual enough times to
  // guarantee that we are in the middle of a linear solve, to verify
  // that we can throw an exception at that point.
  else if (_when == RESIDUAL && _call_no == 3240)
    throw MooseException("MooseException thrown during residual calculation");

  else
    return 0;
}

Real
ExceptionKernel::computeQpJacobian()
{
  // Increment the call number *before* we possibly throw an
  // exception, so this function can continue to be used after that.
  _jac_call_no++;

  if (_when == JACOBIAN && _jac_call_no == 5000)
    throw MooseException("MooseException thrown during Jacobian calculation");

  return 0.;
}
