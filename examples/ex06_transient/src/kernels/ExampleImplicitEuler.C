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

#include "ExampleImplicitEuler.h"

#include "Material.h"

template<>
InputParameters validParams<ExampleImplicitEuler>()
{
  InputParameters params = validParams<ImplicitEuler>();
  params.addParam<Real>("time_coefficient", 1.0, "Time Coefficient");
  return params;
}

ExampleImplicitEuler::ExampleImplicitEuler(const std::string & name,
                                           MooseSystem &sys,
                                           InputParameters parameters)
  :ImplicitEuler(name,sys,parameters),
   // This kernel expects an input parameter named "time_coefficient"
   _time_coefficient(getParam<Real>("time_coefficient"))
{}

Real
ExampleImplicitEuler::computeQpResidual()
{
  return _time_coefficient*ImplicitEuler::computeQpResidual();
}

Real
ExampleImplicitEuler::computeQpJacobian()
{
  return _time_coefficient*ImplicitEuler::computeQpJacobian();
}
