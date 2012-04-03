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

#include "ExampleTimeDerivative.h"

#include "Material.h"

template<>
InputParameters validParams<ExampleTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addParam<Real>("time_coefficient", 1.0, "Time Coefficient");
  return params;
}

ExampleTimeDerivative::ExampleTimeDerivative(const std::string & name,
                                             InputParameters parameters) :
    TimeDerivative(name,parameters),
    // This kernel expects an input parameter named "time_coefficient"
    _time_coefficient(getParam<Real>("time_coefficient"))
{}

Real
ExampleTimeDerivative::computeQpResidual()
{
  return _time_coefficient*TimeDerivative::computeQpResidual();
}

Real
ExampleTimeDerivative::computeQpJacobian()
{
  return _time_coefficient*TimeDerivative::computeQpJacobian();
}
