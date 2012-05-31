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
  return params;
}

ExampleImplicitEuler::ExampleImplicitEuler(const std::string & name,
                                           InputParameters parameters) :
    ImplicitEuler(name,parameters),
    _time_coefficient(getMaterialProperty<Real>("time_coefficient"))
{}

Real
ExampleImplicitEuler::computeQpResidual()
{
  return _time_coefficient[_qp]*ImplicitEuler::computeQpResidual();
}

Real
ExampleImplicitEuler::computeQpJacobian()
{
  return _time_coefficient[_qp]*ImplicitEuler::computeQpJacobian();
}
