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
#include "MooseVariableFEImpl.h"
#include "SystemBase.h"

template <>
InputParameters
validParams<VectorNodalBC>()
{
  return validParams<NodalBCBase>();
}

VectorNodalBC::VectorNodalBC(const InputParameters & parameters)
  : NodalBCBase(parameters),
    MooseVariableInterface<RealVectorValue>(this, true),
    _var(*mooseVariable()),
    _current_node(_var.node()),
    _u(_var.nodalValue())
{
  if (_var.feType().family != LAGRANGE_VEC)
    mooseError("Vector nodal boundary conditions only make sense for LAGRANGE_VEC variables");
  addMooseVariableDependency(mooseVariable());
}

void
VectorNodalBC::computeResidual(NumericVector<Number> & residual)
{
  const std::vector<dof_id_type> & dof_indices = _var.dofIndices();

  RealVectorValue res(0, 0, 0);

  if (!_is_eigen)
    res = computeQpResidual();

  for (size_t i = 0; i < dof_indices.size(); ++i)
    residual.set(dof_indices[i], res(i));
}

void
VectorNodalBC::computeJacobian()
{
  RealVectorValue cached_val = computeQpJacobian();
  const std::vector<dof_id_type> & cached_rows = _var.dofIndices();

  // Cache the user's computeQpJacobian() value for later use.
  for (size_t i = 0; i < cached_rows.size(); ++i)
    _fe_problem.assembly(0).cacheJacobianContribution(
        cached_rows[i], cached_rows[i], cached_val(i));
}

void
VectorNodalBC::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    Real cached_val = computeQpOffDiagJacobian(jvar);
    const std::vector<dof_id_type> & cached_rows = _var.dofIndices();
    // Note: this only works for Lagrange variables...
    dof_id_type cached_col = _current_node->dof_number(_sys.number(), jvar, 0);

    // Cache the user's computeQpJacobian() value for later use.
    for (size_t i = 0; i < cached_rows.size(); ++i)
      _fe_problem.assembly(0).cacheJacobianContribution(cached_rows[i], cached_col, cached_val);
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
