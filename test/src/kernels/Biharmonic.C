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
#include "Biharmonic.h"

template <>
InputParameters
validParams<Biharmonic>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

Biharmonic::Biharmonic(const InputParameters & parameters)
  : Kernel(parameters), _second_u(second()), _second_phi(secondPhi()), _second_test(secondTest())
{
}

Real
Biharmonic::computeQpResidual()
{
  return _second_u[_qp].tr() * _second_test[_i][_qp].tr();
}

Real
Biharmonic::computeQpJacobian()
{
  return _second_phi[_j][_qp].tr() * _second_test[_i][_qp].tr();
}
