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

template <>
InputParameters
validParams<MatDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "the name of the material property we are going to use");

  MooseEnum prop_state("current old older", "current");
  params.addParam<MooseEnum>(
      "prop_state", prop_state, "Declares which property state we should retrieve");
  return params;
}

MatDiffusion::MatDiffusion(const InputParameters & parameters) : Kernel(parameters)
{
  MooseEnum prop_state = getParam<MooseEnum>("prop_state");

  if (prop_state == "current")
    _diff = &getMaterialProperty<Real>("prop_name");
  else if (prop_state == "old")
    _diff = &getMaterialPropertyOld<Real>("prop_name");
  else if (prop_state == "older")
    _diff = &getMaterialPropertyOlder<Real>("prop_name");
}

Real
MatDiffusion::computeQpResidual()
{
  return (*_diff)[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
MatDiffusion::computeQpJacobian()
{
  return (*_diff)[_qp] * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
