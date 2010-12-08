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

#include "ImplicitBackwardDifference2.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<ImplicitBackwardDifference2>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addParam<bool>("start_with_be", true, "Whether or not to use first order Backward Euler for the first timestep");
  return params;
}

ImplicitBackwardDifference2::ImplicitBackwardDifference2(const std::string & name, InputParameters parameters)
  :TimeDerivative(name, parameters)
{
   _moose_system.initTimeSteppingScheme(Moose::BDF2);
}

Real
ImplicitBackwardDifference2::computeQpResidual()
{
  return TimeDerivative::computeQpResidual();
}

Real
ImplicitBackwardDifference2::computeQpJacobian()
{
  return TimeDerivative::computeQpJacobian();
}


