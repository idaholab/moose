/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "JouleHeating.h"

// MOOSE
#include "Function.h"

template<>
InputParameters validParams<JouleHeating>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("potential", "Gradient of the electrical potential");
  return params;
}

JouleHeating::JouleHeating(const InputParameters & parameters) :
    Kernel(parameters),
    _grad_potential(coupledGradient("potential")),
    _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity"))
{
}

Real
JouleHeating::computeQpResidual()
{
  return - _thermal_conductivity[_qp] * _grad_potential[_qp]*_grad_potential[_qp]*_test[_i][_qp];
}


