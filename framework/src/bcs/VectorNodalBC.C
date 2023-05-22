//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorNodalBC.h"

#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "NonlinearSystemBase.h"

InputParameters
VectorNodalBC::validParams()
{
  InputParameters params = NodalBCBase::validParams();
  return params;
}

VectorNodalBC::VectorNodalBC(const InputParameters & parameters)
  : NodalBCBase(parameters),
    MooseVariableInterface<RealVectorValue>(this,
                                            true,
                                            "variable",
                                            Moose::VarKindType::VAR_NONLINEAR,
                                            Moose::VarFieldType::VAR_FIELD_VECTOR),
    _var(*mooseVariable()),
    _current_node(_var.node()),
    _u(_var.nodalValue())
{
  if (_var.feType().family != LAGRANGE_VEC)
    mooseError("Vector nodal boundary conditions only make sense for LAGRANGE_VEC variables");
  addMooseVariableDependency(mooseVariable());
}

void
VectorNodalBC::computeResidual()
{
  const std::vector<dof_id_type> & dof_indices = _var.dofIndices();
  if (dof_indices.empty())
    return;

  const RealVectorValue res = computeQpResidual();

  for (const auto i : index_range(dof_indices))
    setResidual(_sys, res(i), dof_indices[i]);
}

void
VectorNodalBC::computeJacobian()
{
  const std::vector<dof_id_type> & cached_rows = _var.dofIndices();
  if (cached_rows.empty())
    return;

  const RealVectorValue cached_val = computeQpJacobian();

  // Cache the user's computeQpJacobian() value for later use.
  for (const auto i : index_range(cached_rows))
    addJacobianElement(_fe_problem.assembly(0, _sys.number()),
                       cached_val(i),
                       cached_rows[i],
                       cached_rows[i],
                       /*scaling_factor=*/1);
}

void
VectorNodalBC::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    const std::vector<dof_id_type> & cached_rows = _var.dofIndices();
    if (cached_rows.empty())
      return;

    const Real cached_val = computeQpOffDiagJacobian(jvar_num);
    // Note: this only works for Lagrange variables...
    const dof_id_type cached_col = _current_node->dof_number(_sys.number(), jvar_num, 0);

    // Cache the user's computeQpJacobian() value for later use.
    for (const auto i : index_range(cached_rows))
      addJacobianElement(_fe_problem.assembly(0, _sys.number()),
                         cached_val,
                         cached_rows[i],
                         cached_col,
                         /*scaling_factor=*/1);
  }
}

RealVectorValue
VectorNodalBC::computeQpJacobian()
{
  return RealVectorValue(1., 1., 1.);
}

Real
VectorNodalBC::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}
