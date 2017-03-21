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

#include "DotCouplingKernel.h"

template <>
InputParameters
validParams<DotCouplingKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addCoupledVar("v", "Variable being coupled");
  return params;
}

DotCouplingKernel::DotCouplingKernel(const InputParameters & parameters)
  : Kernel(parameters), _v_dot(coupledDot("v")), _dv_dot_dv(coupledDotDu("v"))
{
}

Real
DotCouplingKernel::computeQpResidual()
{
  return -_v_dot[_qp] * _test[_i][_qp];
}

Real
DotCouplingKernel::computeQpJacobian()
{
  return -_dv_dot_dv[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
