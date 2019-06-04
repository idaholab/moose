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

template <>
InputParameters
validParams<CoupledVarNeumannBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredCoupledVar("v", "Coupled variable setting the gradient on the boundary.");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=v$, "
                             "where $v$ is a variable.");
  return params;
}

CoupledVarNeumannBC::CoupledVarNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _coupled_var(coupledValue("v"))
{
}

Real
CoupledVarNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * _coupled_var[_qp];
}
