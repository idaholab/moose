//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassFluxPenaltyBC.h"

registerMooseObject("NavierStokesApp", MassFluxPenaltyBC);

InputParameters
MassFluxPenaltyBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredCoupledVar("u", "The x-velocity");
  params.addRequiredCoupledVar("v", "The y-velocity");
  params.addRequiredParam<unsigned short>("component",
                                          "The velocity component this object is being applied to");
  params.addParam<Real>("gamma", 1, "The penalty to multiply the jump");
  return params;
}

MassFluxPenaltyBC::MassFluxPenaltyBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _vel_x(adCoupledValue("u")),
    _vel_y(adCoupledValue("v")),
    _comp(getParam<unsigned short>("component")),
    _matrix_only(getParam<bool>("matrix_only")),
    _gamma(getParam<Real>("gamma"))
{
}

void
MassFluxPenaltyBC::computeResidual()
{
  if (!_matrix_only)
    ADIntegratedBC::computeResidual();
}

ADReal
MassFluxPenaltyBC::computeQpResidual()
{
  const ADRealVectorValue soln_jump(_vel_x[_qp], _vel_y[_qp], 0);

  return _gamma * soln_jump * _normals[_qp] * _test[_i][_qp] * _normals[_qp](_comp);
}
