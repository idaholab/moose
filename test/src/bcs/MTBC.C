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
#include "MTBC.h"

template <>
InputParameters
validParams<MTBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "the name of the material property we are going to use");
  params.addRequiredParam<Real>("grad", "the value of the gradient");
  return params;
}

MTBC::MTBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _value(getParam<Real>("grad")),
    _mat(getMaterialProperty<Real>("prop_name"))
{
}

Real
MTBC::computeQpResidual()
{
  return -_test[_i][_qp] * _value * _mat[_qp];
}
