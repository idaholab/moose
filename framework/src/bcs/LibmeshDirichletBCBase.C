//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LibmeshDirichletBCBase.h"

#include "Assembly.h"
#include "FEProblemBase.h"
#include "MooseVariableFE.h"
#include "NonlinearSystemBase.h"

#include "libmesh/numeric_vector.h"

InputParameters
LibmeshDirichletBCBase::validParams()
{
  return NodalBC::validParams();
}

LibmeshDirichletBCBase::LibmeshDirichletBCBase(const InputParameters & parameters)
  : NodalBC(parameters)
{
}

std::optional<Real>
LibmeshDirichletBCBase::prescribedValue(const dof_id_type dof) const
{
  const auto & values = _fe_problem.getNonlinearSystemBase(_sys.number()).libmeshDirichletValues();

  if (const auto it = values.find(dof); it != values.end())
    return it->second;

  // libMesh's own constraint machinery already determines this degree of freedom, as an adaptivity
  // hanging node, a periodic partner, or a mode a neighbor's lower p level suppresses. Those
  // constraints are homogeneous, which is precisely what libMesh's constraint enforcement carries,
  // so this boundary condition leaves the row to it.
  if (_sys.dofMap().is_constrained_dof(dof))
    return {};

  mooseError("libMesh's Dirichlet constraint machinery reported no prescribed value for degree of "
             "freedom ",
             dof,
             " of variable '",
             _var.name(),
             "'. It projects the prescribed value over the element sides of a boundary, so this "
             "boundary condition's 'boundary' has to name sidesets rather than nodesets.");
}

void
LibmeshDirichletBCBase::computeResidual()
{
  const auto & dof_indices = _var.dofIndices();

  mooseAssert(_u.size() == dof_indices.size(),
              "Nodal dof values are expected to cover every dof of the node");

  // Every degree of freedom the boundary reaches is pinned, not just the node's first one, so that
  // a HIERARCHIC node carrying several edge modes is fully constrained
  for (const auto i : index_range(dof_indices))
    if (const auto value = prescribedValue(dof_indices[i]))
      setResidual(_sys, _u[i] - *value, dof_indices[i]);
}

void
LibmeshDirichletBCBase::computeJacobian()
{
  const auto & dof_indices = _var.dofIndices();

  for (const auto i : index_range(dof_indices))
    if (prescribedValue(dof_indices[i]))
      addJacobianElement(_fe_problem.assembly(0, _sys.number()),
                         1,
                         dof_indices[i],
                         dof_indices[i],
                         /*scaling_factor=*/1);
}

void
LibmeshDirichletBCBase::computeValue(NumericVector<Number> & current_solution)
{
  const auto & dof_indices = _var.dofIndices();

  for (const auto i : index_range(dof_indices))
    if (const auto value = prescribedValue(dof_indices[i]))
      current_solution.set(dof_indices[i], *value);
}

Real
LibmeshDirichletBCBase::computeQpResidual()
{
  mooseError("This boundary condition sources its prescribed values by projection over the whole "
             "boundary rather than pointwise, so it has no per-node value to compute.");
}
