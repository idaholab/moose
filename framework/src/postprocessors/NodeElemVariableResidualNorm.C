//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodeElemVariableResidualNorm.h"
#include "MooseVariableFieldBase.h"
#include "NonlinearSystemBase.h"

#include "libmesh/dof_map.h"
#include "libmesh/node_elem.h"

registerMooseObject("MooseApp", NodeElemVariableResidualNorm);

InputParameters
NodeElemVariableResidualNorm::validParams()
{
  InputParameters params = VariableResidualNormBase::validParams();

  params.addClassDescription(
      "Computes the residual norm (absolute value) of a NodeElem residual for a variable.");

  return params;
}

NodeElemVariableResidualNorm::NodeElemVariableResidualNorm(const InputParameters & parameters)
  : VariableResidualNormBase(parameters)
{
}

std::vector<dof_id_type>
NodeElemVariableResidualNorm::getCurrentElemDofIndices() const
{
  mooseAssert(dynamic_cast<const libMesh::NodeElem *>(_current_elem),
              "This should only be used for a NodeElem block.");

  std::vector<dof_id_type> dof_indices;
  _var.dofMap().dof_indices(_current_elem, dof_indices, _var.number());
  return dof_indices;
}

void
NodeElemVariableResidualNorm::computeNorm()
{
  // compute the total number of Dofs for the variable on the subdomain
  auto n_dofs = _local_dof_indices.size();
  gatherSum(n_dofs);
  mooseAssert(n_dofs == 1, "There should be exactly one degree of freedom.");

  if (_local_dof_indices.size())
    _norm = std::abs(_nl_residual_vector(*_local_dof_indices.begin()));
  gatherSum(_norm);

  if (!_include_scaling_factor)
    _norm /= _var.scalingFactor();
}
