//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DirichletBCBase.h"

InputParameters
DirichletBCBase::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addParam<bool>(
      "preset", true, "Whether or not to preset the BC (apply the value before the solve begins).");
  return params;
}

DirichletBCBase::DirichletBCBase(const InputParameters & parameters)
  : NodalBC(parameters), _preset(getParam<bool>("preset"))
{
}

void
DirichletBCBase::computeValue(NumericVector<Number> & current_solution)
{
  mooseAssert(_preset, "BC is not preset");

  if (_var.isNodalDefined())
  {
    const dof_id_type & dof_idx = _var.nodalDofIndex();
    current_solution.set(dof_idx, computeQpValue());
  }
}

Real
DirichletBCBase::computeQpResidual()
{
  return _u[_qp] - computeQpValue();
}
