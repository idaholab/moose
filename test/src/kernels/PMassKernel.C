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
#include "PMassKernel.h"

template <>
InputParameters
validParams<PMassKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addRangeCheckedParam<Real>("p", 2.0, "p>=1.0", "The actual exponent is p-2");
  return params;
}

PMassKernel::PMassKernel(const InputParameters & parameters)
  : Kernel(parameters), _p(getParam<Real>("p") - 2.0)
{
}

Real
PMassKernel::computeQpResidual()
{
  return std::pow(std::fabs(_u[_qp]), _p) * _u[_qp] * _test[_i][_qp];
}

Real
PMassKernel::computeQpJacobian()
{
  // Note: this jacobian evaluation is not exact when p!=2.
  return std::pow(std::fabs(_phi[_j][_qp]), _p) * _phi[_j][_qp] * _test[_i][_qp];
}
