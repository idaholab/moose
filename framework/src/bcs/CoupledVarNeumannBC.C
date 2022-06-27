//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledVarNeumannBC.h"

registerMooseObject("MooseApp", CoupledVarNeumannBC);

InputParameters
CoupledVarNeumannBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredCoupledVar("v", "Coupled variable setting the gradient on the boundary.");
  params.addCoupledVar("scale_factor", 1., "Scale factor to multiply the heat flux with");
  params.addParam<Real>(
      "coef", 1.0, "Coefficent ($\\sigma$) multiplier for the coupled force term.");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=v$, "
                             "where $v$ is a variable.");
  return params;
}

CoupledVarNeumannBC::CoupledVarNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _coupled_var(coupledValue("v")),
    _coupled_num(coupled("v")),
    _coef(getParam<Real>("coef")),
    _scale_factor(coupledValue("scale_factor"))
{
}

Real
CoupledVarNeumannBC::computeQpResidual()
{
  return -_scale_factor[_qp] * _coef * _test[_i][_qp] * _coupled_var[_qp];
}

Real
CoupledVarNeumannBC::computeQpOffDiagJacobian(const unsigned int jvar)
{
  if (jvar == _coupled_num)
    return -_scale_factor[_qp] * _coef * _test[_i][_qp] * _phi[_j][_qp];
  else
    return 0;
}
