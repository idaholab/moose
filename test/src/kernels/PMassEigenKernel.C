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
#include "PMassEigenKernel.h"

template <>
InputParameters
validParams<PMassEigenKernel>()
{
  InputParameters params = validParams<EigenKernel>();
  params.addRangeCheckedParam<Real>("p", 2.0, "p>=1.0", "The exponent p");
  return params;
}

PMassEigenKernel::PMassEigenKernel(const InputParameters & parameters)
  : EigenKernel(parameters), _p(getParam<Real>("p") - 2.0)
{
}

Real
PMassEigenKernel::computeQpResidual()
{
  return -std::pow(std::fabs(_u[_qp]), _p) * _u[_qp] * _test[_i][_qp];
}

Real
PMassEigenKernel::computeQpJacobian()
{
  // Note: this jacobian evaluation is not exact when p!=2.
  return -std::pow(std::fabs(_phi[_j][_qp]), _p) * _phi[_j][_qp] * _test[_i][_qp];
}
