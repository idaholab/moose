//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNodeElemConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseEnum.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/string_to_enum.h"

InputParameters
ADNodeElemConstraint::validParams()
{
  InputParameters params = Constraint::validParams();
  params.addRequiredParam<SubdomainName>("secondary", "secondary block id");
  params.addRequiredParam<SubdomainName>("primary", "primary block id");
  params.addRequiredCoupledVar("primary_variable",
                               "The variable on the primary side of the domain");

  return params;
}

ADNodeElemConstraint::ADNodeElemConstraint(const InputParameters & parameters)
  : Constraint(parameters),
    // The secondary side is at nodes (hence passing 'true').  The neighbor side is the primary side
    // and it is not at nodes (so passing false)
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, true, false),
    NeighborMooseVariableInterface<Real>(
        this, true, Moose::VarKindType::VAR_SOLVER, Moose::VarFieldType::VAR_FIELD_STANDARD),

    _secondary(_mesh.getSubdomainID(getParam<SubdomainName>("secondary"))),
    _primary(_mesh.getSubdomainID(getParam<SubdomainName>("primary"))),
    _var(_sys.getFieldVariable<Real>(_tid, parameters.get<NonlinearVariableName>("variable"))),

    _primary_q_point(_assembly.adQPoints()),
    _primary_qrule(_assembly.qRule()),

    _current_node(_var.node()),
    _current_elem(_var.neighbor()),

    _u_secondary(_var.adDofValues()),
    _u_secondary_old(_var.dofValuesOld()),
    _phi_secondary(1),
    _test_secondary(1), // One entry

    _primary_var(*getVar("primary_variable", 0)),
    _primary_var_num(_primary_var.number()),

    _phi_primary(_assembly.phiNeighbor(_primary_var)),
    _grad_phi_primary(_assembly.gradPhiNeighbor(_primary_var)),

    _test_primary(_var.phiNeighbor()),
    _grad_test_primary(_var.gradPhiNeighbor()),

    _u_primary(_primary_var.adSlnNeighbor()),
    _u_primary_old(_primary_var.slnOldNeighbor()),
    _grad_u_primary(_primary_var.adGradSlnNeighbor()),

    _dof_map(_sys.dofMap()),
    _node_to_elem_map(_mesh.nodeToElemMap()),

    _overwrite_secondary_residual(false)
{
  _mesh.errorIfDistributedMesh("ADNodeElemConstraint");

  addMooseVariableDependency(&_var);
  // Put a "1" into test_secondary
  // will always only have one entry that is 1
  _test_secondary[0].push_back(1);
}

ADNodeElemConstraint::~ADNodeElemConstraint()
{
  _phi_secondary.release();
  _test_secondary.release();
}

void
ADNodeElemConstraint::computeSecondaryValue(NumericVector<Number> & current_solution)
{
  const dof_id_type & dof_idx = _var.nodalDofIndex();
  _qp = 0;
  current_solution.set(dof_idx, computeQpSecondaryValue());
}

void
ADNodeElemConstraint::computeResidual()
{

  _qp = 0;

  _residuals.resize(_test_primary.size(), 0);
  for (auto & r : _residuals)
    r = 0;

  for (_i = 0; _i < _test_primary.size(); _i++)
    _residuals[_i] += raw_value(computeQpResidual(Moose::Primary));

  addResiduals(
      _assembly, _residuals, _primary_var.dofIndicesNeighbor(), _primary_var.scalingFactor());

  _residuals.resize(_test_secondary.size(), 0);

  for (auto & r : _residuals)
    r = 0;

  for (_i = 0; _i < _test_secondary.size(); _i++)
    _residuals[_i] += raw_value(computeQpResidual(Moose::Secondary));

  addResiduals(_assembly, _residuals, _var.dofIndices(), _var.scalingFactor());
}

void
ADNodeElemConstraint::computeJacobian()
{
  _qp = 0;

  std::vector<ADReal> primary_residual(_test_primary.size(), 0);

  for (_i = 0; _i < _test_primary.size(); _i++)
    primary_residual[_i] += computeQpResidual(Moose::Primary);

  addJacobian(_assembly, primary_residual, _primary_var.dofIndicesNeighbor(), _var.scalingFactor());

  std::vector<ADReal> secondary_residual(_test_secondary.size(), 0);

  for (_i = 0; _i < _test_secondary.size(); _i++)
    secondary_residual[_i] += computeQpResidual(Moose::Secondary);

  addJacobian(_assembly, secondary_residual, _var.dofIndices(), _var.scalingFactor());
}

void
ADNodeElemConstraint::computeOffDiagJacobian(const unsigned int /*jvar_num*/)
{
}

void
ADNodeElemConstraint::getConnectedDofIndices(unsigned int var_num)
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
ADNodeElemConstraint::overwriteSecondaryResidual()
{
  return _overwrite_secondary_residual;
}
