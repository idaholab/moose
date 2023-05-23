//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodeElemConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseEnum.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/string_to_enum.h"

InputParameters
NodeElemConstraint::validParams()
{
  InputParameters params = Constraint::validParams();
  params.addRequiredParam<SubdomainName>("secondary", "secondary block id");
  params.addRequiredParam<SubdomainName>("primary", "primary block id");
  params.addRequiredCoupledVar("primary_variable",
                               "The variable on the primary side of the domain");

  return params;
}

NodeElemConstraint::NodeElemConstraint(const InputParameters & parameters)
  : Constraint(parameters),
    // The secondary side is at nodes (hence passing 'true').  The neighbor side is the primary side
    // and it is not at nodes (so passing false)
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, true, false),
    NeighborMooseVariableInterface<Real>(
        this, true, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD),

    _secondary(_mesh.getSubdomainID(getParam<SubdomainName>("secondary"))),
    _primary(_mesh.getSubdomainID(getParam<SubdomainName>("primary"))),
    _var(_sys.getFieldVariable<Real>(_tid, parameters.get<NonlinearVariableName>("variable"))),

    _primary_q_point(_assembly.qPoints()),
    _primary_qrule(_assembly.qRule()),

    _current_node(_var.node()),
    _current_elem(_var.neighbor()),

    _u_secondary(_var.dofValues()),
    _u_secondary_old(_var.dofValuesOld()),
    _phi_secondary(1),
    _test_secondary(1), // One entry

    _primary_var(*getVar("primary_variable", 0)),
    _primary_var_num(_primary_var.number()),

    _phi_primary(_assembly.phiNeighbor(_primary_var)),
    _grad_phi_primary(_assembly.gradPhiNeighbor(_primary_var)),

    _test_primary(_var.phiNeighbor()),
    _grad_test_primary(_var.gradPhiNeighbor()),

    _u_primary(_primary_var.slnNeighbor()),
    _u_primary_old(_primary_var.slnOldNeighbor()),
    _grad_u_primary(_primary_var.gradSlnNeighbor()),

    _dof_map(_sys.dofMap()),
    _node_to_elem_map(_mesh.nodeToElemMap()),

    _overwrite_secondary_residual(false)
{
  _mesh.errorIfDistributedMesh("NodeElemConstraint");

  addMooseVariableDependency(&_var);
  // Put a "1" into test_secondary
  // will always only have one entry that is 1
  _test_secondary[0].push_back(1);
}

NodeElemConstraint::~NodeElemConstraint()
{
  _phi_secondary.release();
  _test_secondary.release();
}

void
NodeElemConstraint::computeSecondaryValue(NumericVector<Number> & current_solution)
{
  const dof_id_type & dof_idx = _var.nodalDofIndex();
  _qp = 0;
  current_solution.set(dof_idx, computeQpSecondaryValue());
}

void
NodeElemConstraint::computeResidual()
{
  _qp = 0;

  prepareVectorTagNeighbor(_assembly, _var.number());
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

bool
NodeElemConstraint::overwriteSecondaryResidual()
{
  return _overwrite_secondary_residual;
}
