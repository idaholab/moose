//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodeFaceConstraintBase.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseEnum.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "PenetrationLocator.h"
#include "SystemBase.h"

#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<NodeFaceConstraintBase>()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");
  InputParameters params = validParams<Constraint>();
  params.addRequiredParam<BoundaryName>("slave", "The boundary ID associated with the slave side");
  params.addRequiredParam<BoundaryName>("master",
                                        "The boundary ID associated with the master side");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>(
      "normal_smoothing_distance",
      "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method",
                               "Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order used for projections");

  params.addRequiredCoupledVar("master_variable", "The variable on the master side of the domain");

  return params;
}

NodeFaceConstraintBase::NodeFaceConstraintBase(const InputParameters & parameters)
  : Constraint(parameters),
    // The slave side is at nodes (hence passing 'true').  The neighbor side is the master side and
    // it is not at nodes (so passing false)
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, true, false),
    NeighborMooseVariableInterface<Real>(
        this, true, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _slave(_mesh.getBoundaryID(getParam<BoundaryName>("slave"))),
    _master(_mesh.getBoundaryID(getParam<BoundaryName>("master"))),

    _master_q_point(_assembly.qPointsFace()),
    _master_qrule(_assembly.qRuleFace()),

    _penetration_locator(
        getPenetrationLocator(getParam<BoundaryName>("master"),
                              getParam<BoundaryName>("slave"),
                              Utility::string_to_enum<Order>(getParam<MooseEnum>("order")))),

    _current_node(_var.node()),
    _current_master(_var.neighbor()),
    _test_slave(1), // One entry

    _master_var(*getVar("master_variable", 0)),
    _master_var_num(_master_var.number()),

    _test_master(_var.phiFaceNeighbor()),
    _grad_test_master(_var.gradPhiFaceNeighbor()),

    _dof_map(_sys.dofMap()),
    _node_to_elem_map(_mesh.nodeToElemMap()),

    _overwrite_slave_residual(true)
{
  addMooseVariableDependency(&_var);

  if (parameters.isParamValid("tangential_tolerance"))
  {
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));
  }
  if (parameters.isParamValid("normal_smoothing_distance"))
  {
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));
  }
  if (parameters.isParamValid("normal_smoothing_method"))
  {
    _penetration_locator.setNormalSmoothingMethod(
        parameters.get<std::string>("normal_smoothing_method"));
  }
  // Put a "1" into test_slave
  // will always only have one entry that is 1
  _test_slave[0].push_back(1);
}

NodeFaceConstraintBase::~NodeFaceConstraintBase() { _test_slave.release(); }

void
NodeFaceConstraintBase::computeSlaveValue(NumericVector<Number> & current_solution)
{
  dof_id_type & dof_idx = _var.nodalDofIndex();
  _qp = 0;
  current_solution.set(dof_idx, computeQpSlaveValue());
}

void
NodeFaceConstraintBase::getConnectedDofIndices(unsigned int var_num)
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
}

bool
NodeFaceConstraintBase::overwriteSlaveResidual()
{
  return _overwrite_slave_residual;
}
