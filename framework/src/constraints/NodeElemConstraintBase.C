//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodeElemConstraintBase.h"

#include "MooseVariableFE.h"
#include "SystemBase.h"

InputParameters
NodeElemConstraintBase::validParams()
{
  InputParameters params = Constraint::validParams();
  params.addRequiredParam<SubdomainName>("secondary", "secondary block id");
  params.addRequiredParam<SubdomainName>("primary", "primary block id");
  params.addRequiredCoupledVar("primary_variable",
                               "The variable on the primary side of the domain");
  return params;
}

NodeElemConstraintBase::NodeElemConstraintBase(const InputParameters & parameters)
  : Constraint(parameters),
    // The secondary side is at nodes (hence passing 'true').  The neighbor side is the primary side
    // and it is not at nodes (so passing false)
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, true, false),
    NeighborMooseVariableInterface<Real>(
        this, true, Moose::VarKindType::VAR_SOLVER, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _var(_sys.getFieldVariable<Real>(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _primary_var(*getVar("primary_variable", 0)),

    _secondary(_mesh.getSubdomainID(getParam<SubdomainName>("secondary"))),
    _primary(_mesh.getSubdomainID(getParam<SubdomainName>("primary"))),

    _current_node(_var.node()),
    _current_elem(_var.neighbor()),

    _phi_secondary(1),
    _test_secondary(1), // One entry

    _phi_primary(_assembly.phiNeighbor(_primary_var)),
    _test_primary(_var.phiNeighbor()),

    _node_to_elem_map(_mesh.nodeToElemMap()),
    _overwrite_secondary_residual(false)
{
  addMooseVariableDependency(&_var);
  _mesh.errorIfDistributedMesh("NodeElemConstraintBase");
  // Put a "1" into test_secondary
  // will always only have one entry that is 1
  _test_secondary[0].push_back(1);
}

NodeElemConstraintBase::~NodeElemConstraintBase()
{
  _phi_secondary.release();
  _test_secondary.release();
}

void
NodeElemConstraintBase::computeSecondaryValue(NumericVector<Number> & current_solution)
{
  const dof_id_type & dof_idx = _var.nodalDofIndex();
  _qp = 0;
  current_solution.set(dof_idx, computeQpSecondaryValue());
}

bool
NodeElemConstraintBase::overwriteSecondaryResidual()
{
  return _overwrite_secondary_residual;
}
