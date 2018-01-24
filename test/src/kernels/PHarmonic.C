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
#include "PHarmonic.h"

template <>
InputParameters
validParams<PHarmonic>()
{
  InputParameters params = validParams<Kernel>();
  params.addRangeCheckedParam<Real>("p", 2.0, "p>=1.0", "The exponent p");
  return params;
}

PHarmonic::PHarmonic(const InputParameters & parameters)
  : Kernel(parameters), _p(getParam<Real>("p") - 2.0)
{
}

Real
PHarmonic::computeQpResidual()
{
  Real t = std::sqrt(_grad_u[_qp] * _grad_u[_qp]);
  return _grad_test[_i][_qp] * _grad_u[_qp] * std::pow(t, _p);
}

Real
PHarmonic::computeQpJacobian()
{
  // Note: this jacobian evaluation is not exact when p!=2.
  Real t = std::sqrt(_grad_phi[_j][_qp] * _grad_phi[_j][_qp]);
  return _grad_test[_i][_qp] * _grad_phi[_j][_qp] * std::pow(t, _p);
}
