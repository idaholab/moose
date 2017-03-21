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
#include "CoupledEigenKernel.h"

template <>
InputParameters
validParams<CoupledEigenKernel>()
{
  InputParameters params = validParams<EigenKernel>();
  params.addRequiredCoupledVar("v", "Variable to be coupled in");
  return params;
}

CoupledEigenKernel::CoupledEigenKernel(const InputParameters & parameters)
  : EigenKernel(parameters), _v(coupledValue("v"))
{
}

Real
CoupledEigenKernel::computeQpResidual()
{
  return -_v[_qp] * _test[_i][_qp];
}
