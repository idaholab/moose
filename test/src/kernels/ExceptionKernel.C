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
    _call_no(0)
{
}

Real
ExceptionKernel::computeQpResidual()
{
  if (_when == INITIAL_CONDITION)
    throw MooseException("Initial error");
  else if (_when == RESIDUAL)
  {
    if (_call_no == 3240)                 // 1000 calls to computeQpResidual is enough to get us into linear solve
    {
      std::cout<<"Stopping solve!"<<std::endl;
      _fe_problem.getNonlinearSystem().stopSolve();
    }

      //throw MooseException("Residual error");
  }
  _call_no++;

  return 0;
}

Real
ExceptionKernel::computeQpJacobian()
{
  if (_when == JACOBIAN)
    throw MooseException("Jacobian");

  return 0.;
}
