//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodeElemConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseMesh.h"

InputParameters
NodeElemConstraint::validParams()
{
  InputParameters params = NodeElemConstraintBase::validParams();
  return params;
}

NodeElemConstraint::NodeElemConstraint(const InputParameters & parameters)
  : NodeElemConstraintBase(parameters),
    _u_primary(_primary_var.slnNeighbor()),
    _u_secondary(_var.dofValues()),
    _grad_phi_primary(_assembly.gradPhiNeighbor(_primary_var)),
    _grad_test_primary(_var.gradPhiNeighbor()),
    _grad_u_primary(_primary_var.gradSlnNeighbor())
{
  if (_primary_var.number() != _var.number())
    paramError("primary_variable", "The primary_variable and variable must be the same.");
}

void
NodeElemConstraint::computeResidual()
{
  _qp = 0;

  prepareVectorTagNeighbor(_assembly, _primary_var.number());
  for (_i = 0; _i < _test_primary.size(); _i++)
    _local_re(_i) += computeQpResidual(Moose::Primary);
  accumulateTaggedLocalResidual();

  prepareVectorTag(_assembly, _var.number());
  for (_i = 0; _i < _test_secondary.size(); _i++)
    _local_re(_i) += computeQpResidual(Moose::Secondary);
  accumulateTaggedLocalResidual();
}

void
NodeElemConstraint::computeJacobian()
{
  getConnectedDofIndices(_var.number());

  _Kee.resize(_test_secondary.size(), _connected_dof_indices.size());
  _Kne.resize(_test_primary.size(), _connected_dof_indices.size());

  for (_i = 0; _i < _test_secondary.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpJacobian(Moose::SecondarySecondary);

  prepareMatrixTagNeighbor(_assembly, _var.number(), _var.number(), Moose::ElementNeighbor);
  if (_local_ke.m() && _local_ke.n())
    for (_i = 0; _i < _test_secondary.size(); _i++)
      for (_j = 0; _j < _phi_primary.size(); _j++)
        _local_ke(_i, _j) += computeQpJacobian(Moose::SecondaryPrimary);
  accumulateTaggedLocalMatrix();

  for (_i = 0; _i < _test_primary.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kne(_i, _j) += computeQpJacobian(Moose::PrimarySecondary);

  prepareMatrixTagNeighbor(
      _assembly, _primary_var.number(), _var.number(), Moose::NeighborNeighbor);
  if (_local_ke.m() && _local_ke.n())
    for (_i = 0; _i < _test_primary.size(); _i++)
      for (_j = 0; _j < _phi_primary.size(); _j++)
        _local_ke(_i, _j) += computeQpJacobian(Moose::PrimaryPrimary);
  accumulateTaggedLocalMatrix();
}

void
NodeElemConstraint::computeOffDiagJacobian(const unsigned int jvar_num)
{
  getConnectedDofIndices(jvar_num);

  _Kee.resize(_test_secondary.size(), _connected_dof_indices.size());
  _Kne.resize(_test_primary.size(), _connected_dof_indices.size());

  for (_i = 0; _i < _test_secondary.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpOffDiagJacobian(Moose::SecondarySecondary, jvar_num);

  prepareMatrixTagNeighbor(_assembly, _var.number(), jvar_num, Moose::ElementNeighbor);
  for (_i = 0; _i < _test_secondary.size(); _i++)
    for (_j = 0; _j < _phi_primary.size(); _j++)
      _local_ke(_i, _j) += computeQpOffDiagJacobian(Moose::SecondaryPrimary, jvar_num);
  accumulateTaggedLocalMatrix();

  if (_Kne.m() && _Kne.n())
    for (_i = 0; _i < _test_primary.size(); _i++)
      // Loop over the connected dof indices so we can get all the jacobian contributions
      for (_j = 0; _j < _connected_dof_indices.size(); _j++)
        _Kne(_i, _j) += computeQpOffDiagJacobian(Moose::PrimarySecondary, jvar_num);

  prepareMatrixTagNeighbor(_assembly, _primary_var.number(), jvar_num, Moose::NeighborNeighbor);
  for (_i = 0; _i < _test_primary.size(); _i++)
    for (_j = 0; _j < _phi_primary.size(); _j++)
      _local_ke(_i, _j) += computeQpOffDiagJacobian(Moose::PrimaryPrimary, jvar_num);
  accumulateTaggedLocalMatrix();
}

void
NodeElemConstraint::getConnectedDofIndices(unsigned int var_num)
{
  MooseVariableFEBase & var = _sys.getVariable(0, var_num);

  _connected_dof_indices.clear();
  std::set<dof_id_type> unique_dof_indices;

  auto node_to_elem_pair = _node_to_elem_map.find(_current_node->id());
  mooseAssert(node_to_elem_pair != _node_to_elem_map.end(), "Missing entry in node to elem map");
  const std::vector<dof_id_type> & elems = node_to_elem_pair->second;

  // Get the dof indices from each elem connected to the node
  for (const auto & cur_elem : elems)
  {
    std::vector<dof_id_type> dof_indices;

    var.getDofIndices(_mesh.elemPtr(cur_elem), dof_indices);

    for (const auto & dof : dof_indices)
      unique_dof_indices.insert(dof);
  }

  for (const auto & dof : unique_dof_indices)
    _connected_dof_indices.push_back(dof);

  _phi_secondary.resize(_connected_dof_indices.size());

  const dof_id_type current_node_var_dof_index = _sys.getVariable(0, var_num).nodalDofIndex();

  // Fill up _phi_secondary so that it is 1 when j corresponds to the dof associated with this node
  // and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  _qp = 0;
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_secondary[j].resize(1);

    if (_connected_dof_indices[j] == current_node_var_dof_index)
      _phi_secondary[j][_qp] = 1.0;
    else
      _phi_secondary[j][_qp] = 0.0;
  }
}

Real
NodeElemConstraint::computeQpJacobian(Moose::ConstraintJacobianType /*type*/)
{
  mooseError("Derived classes must implement computeQpJacobian.");
  return 0;
}
