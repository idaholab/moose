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

// MOOSE includes
#include "ScalarCoupledPostprocessor.h"

template <>
InputParameters
validParams<ScalarCoupledPostprocessor>()
{
  InputParameters params = validParams<SideIntegralPostprocessor>();
  params.addRequiredCoupledVar("variable", "Name of variable");
  params.addRequiredCoupledVar("coupled_scalar", "The name of the scalar coupled variable");
  return params;
}

ScalarCoupledPostprocessor::ScalarCoupledPostprocessor(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _coupled_scalar(coupledScalarValue("coupled_scalar")),
    _u(coupledValue("variable"))
{
}

Real
ScalarCoupledPostprocessor::computeQpIntegral()
{
  return _coupled_scalar[0] - _u[_qp];
}
