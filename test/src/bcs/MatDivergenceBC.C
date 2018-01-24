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
#include "MatDivergenceBC.h"

template <>
InputParameters
validParams<MatDivergenceBC>()
{
  InputParameters params = validParams<DivergenceBC>();
  params.addRequiredParam<MaterialPropertyName>("prop_name", "The name of the material property");

  return params;
}

MatDivergenceBC::MatDivergenceBC(const InputParameters & parameters)
  : DivergenceBC(parameters), _mat(getMaterialProperty<Real>("prop_name"))
{
}

MatDivergenceBC::~MatDivergenceBC() {}

Real
MatDivergenceBC::computeQpResidual()
{
  return _mat[_qp] * DivergenceBC::computeQpResidual();
}

Real
MatDivergenceBC::computeQpJacobian()
{
  return _mat[_qp] * DivergenceBC::computeQpJacobian();
}
