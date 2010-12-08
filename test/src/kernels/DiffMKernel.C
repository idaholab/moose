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

#include "DiffMKernel.h"

template<>
InputParameters validParams<DiffMKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("mat_prop", "the name of the material property we are going to use");
  return params;
}


DiffMKernel::DiffMKernel(const std::string & name, InputParameters parameters)
  : Kernel(name, parameters),
    _prop_name(getParam<std::string>("mat_prop")),
    _diff(getMaterialProperty<Real>(_prop_name)),
    _vec_prop(getMaterialProperty<std::vector<Real> >("vector_property"))
{
}

Real
DiffMKernel::computeQpResidual()
{
  return _diff[_qp] * _grad_test[_i][_qp] * _grad_u[_qp] - 4.0;
}

Real
DiffMKernel::computeQpJacobian()
{
  return _diff[_qp] * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
