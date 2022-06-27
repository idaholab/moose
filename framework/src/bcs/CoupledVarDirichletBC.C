//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledVarDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", CoupledVarDirichletBC);

InputParameters
CoupledVarDirichletBC::validParams()
{
  InputParameters params = DirichletBCBase::validParams();
  params.addRequiredCoupledVar("v", "Coupled variable setting the value on the boundary.");
  params.addParam<FunctionName>("scale_factor", 1., "Scale factor to multiply the boundary value with");
  params.addClassDescription("Imposes the Dirichlet boundary condition $u=v$, where $u$ is the equation variable and $v$ is another variable"
                             "is a variable.");
  return params;
}

CoupledVarDirichletBC::CoupledVarDirichletBC(const InputParameters & parameters)
  : DirichletBCBase(parameters),
    _coupled_var(coupledValue("v")),
    _coupled_num(coupled("v")),
    _scale_factor(getFunction("scale_factor"))
{
}

Real
CoupledVarDirichletBC::computeQpValue()
{
  return _scale_factor.value(_t, *_current_node) * _coupled_var[_qp];
}

Real
CoupledVarDirichletBC::computeQpJacobian(const unsigned int jvar)
{
  if (jvar == _coupled_num)
    return _scale_factor.value(_t, *_current_node);
  else
    return 0;
}

Real
CoupledVarDirichletBC::computeQpOffDiagJacobian(const unsigned int jvar)
{
  if (jvar == _coupled_num)
    return _scale_factor.value(_t, *_current_node);
  else
    return 0;
}
