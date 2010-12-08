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

template<>
InputParameters validParams<MTBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addRequiredParam<std::string>("prop_name", "the name of the material property we are going to use");
  params.addRequiredParam<Real>("grad", "the value of the gradient");
  return params;
}

MTBC::MTBC(const std::string & name, InputParameters parameters)
  :BoundaryCondition(name, parameters),
    _value(getParam<Real>("grad")),
    _prop_name(getParam<std::string>("prop_name")),
    _mat(getMaterialProperty<Real>(_prop_name))
{
}

Real
MTBC::computeQpResidual()
{
  return -_test[_i][_qp]*_value*_mat[_qp];
}

