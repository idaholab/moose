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

#include "MatDiffusion.h"

template<>
InputParameters validParams<MatDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("prop_name", "the name of the material property we are going to use");
  return params;
}


MatDiffusion::MatDiffusion(const std::string & name, InputParameters parameters)
  : Kernel(name, parameters),
    _prop_name(getParam<std::string>("prop_name")),
    _diff(getMaterialProperty<Real>(_prop_name))
{
}

Real
MatDiffusion::computeQpResidual()
{
  return _diff[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
MatDiffusion::computeQpJacobian()
{
  return _diff[_qp] * _grad_test[_i][_qp] * _grad_test[_j][_qp];
}
