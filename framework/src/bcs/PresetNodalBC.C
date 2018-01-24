//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PresetNodalBC.h"

// MOOSE includes
#include "MooseVariable.h"

#include "libmesh/numeric_vector.h"

template <>
InputParameters
validParams<PresetNodalBC>()
{
  InputParameters p = validParams<NodalBC>();
  return p;
}

PresetNodalBC::PresetNodalBC(const InputParameters & parameters) : NodalBC(parameters) {}

void
PresetNodalBC::computeValue(NumericVector<Number> & current_solution)
{
  dof_id_type & dof_idx = _var.nodalDofIndex();
  _qp = 0;
  current_solution.set(dof_idx, computeQpValue());
}

Real
PresetNodalBC::computeQpResidual()
{
  return _u[_qp] - computeQpValue();
}
