//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledVarDirichletBC.h"

registerMooseObject("MooseApp", CoupledVarDirichletBC);

InputParameters
CoupledVarDirichletBC::validParams()
{
  InputParameters params = DirichletBCBase::validParams();
  params.addRequiredCoupledVar("v", "Coupled variable setting the value on the boundary.");
  params.addParam<FunctorName>("scale_factor", 1., "Scale factor to multiply the heat flux with");
  params.addClassDescription("Imposes the essential boundary condition $u=v$, where $v$ "
                             "is a variable.");
  return params;
}

CoupledVarDirichletBC::CoupledVarDirichletBC(const InputParameters & parameters)
  : DirichletBCBase(parameters),
    _coupled_var(coupledValue("v")),
    _coupled_num(coupled("v")),
    _coef(getParam<Real>("coef")),
    _scale_factor(coupledValue("scale_factor"))
{
}

Real
CoupledVarDirichletBC::computeQpValue()
{
  return  _coupled_var[_qp];
}
