//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPressurePin.h"
#include "MooseMesh.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "libmesh/node.h"
#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/dof_map.h"
#include "libmesh/numeric_vector.h"

registerMooseObject("NavierStokesApp", FVPressurePin);

InputParameters
FVPressurePin::validParams()
{
  InputParameters params = NodalBCBase::validParams();
  params.addRequiredParam<Real>("value", "Value of the BC");
  params.declareControllable("value");
  params.addClassDescription("Imposes a pressure pin constraint.");
  return params;
}

void
FVPressurePin::boundaryParamError()
{
  paramError("boundary",
             "The FVPressurePin class should only be associated with a single boundary node.");
}

FVPressurePin::FVPressurePin(const InputParameters & parameters)
  : NodalBCBase(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_NONLINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _var(*mooseVariableFV()),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _mesh(getCheckedPointerParam<SubProblem *>("_subproblem")->mesh()),
    _value(getParam<Real>("value"))
{
  addMooseVariableDependency(&_var);

  const auto & boundary_names = getParam<std::vector<BoundaryName>>("boundary");

  if (boundary_names.size() != 1)
    boundaryParamError();

  const auto boundary_id = _mesh.getBoundaryID(boundary_names[0]);

  const auto & nodeset_to_nodes = _mesh.nodeSetNodes();

  auto it = nodeset_to_nodes.find(boundary_id);

  const std::vector<dof_id_type> * nodes = nullptr;
  bool local_node = false;

  if (it != nodeset_to_nodes.end())
    nodes = &it->second;

  if (nodes && nodes->size() > 1)
    paramError("boundary",
               "The FVPressurePin class should only be associated with a single boundary node.");

  const Node * const one_node = nodes ? _mesh.getMesh().query_node_ptr((*nodes)[0]) : nullptr;
  mooseAssert(!nodes || one_node,
              "It's weird that this nodeid existed in our map, but we can't query it on the mesh.");
  mooseAssert(!nodes || one_node->id() == (*nodes)[0], "the ids should match");

  local_node = nodes && one_node->processor_id() == this->processor_id();

  std::vector<dof_id_type> global_nodes;
  if (local_node)
    global_nodes.push_back((*nodes)[0]);

  _communicator.gather(0, global_nodes);

  if (_communicator.rank() == 0)
    if (global_nodes.size() != 1)
      boundaryParamError();

  // lindsayad: No matter how we refine, this node should still exist and be executed on (right?)
  _node_id = local_node ? (*nodes)[0] : libMesh::DofObject::invalid_id;
}

dof_id_type
FVPressurePin::getDofIndex()
{
  const auto & node_to_elem = _mesh.nodeToElemMap();

  auto it = node_to_elem.find(_node_id);

  mooseAssert(it != node_to_elem.end(), "Couldn't find the current node id");

  const auto & elem_ids = it->second;

  mooseAssert(elem_ids.size(), "there should be some elements connected to this node");

  // Just pick the first element
  const Elem * const elem_to_execute = _mesh.getMesh().elem_ptr(elem_ids.front());

  std::vector<dof_id_type> dof_indices;

  _sys.dofMap().dof_indices(elem_to_execute, dof_indices, _var.number());

  mooseAssert(dof_indices.size() == 1,
              "How did you manage to use something other than a constant monomial?");

  return dof_indices.front();
}

void
FVPressurePin::computeResidual()
{
  if (_node_id == libMesh::DofObject::invalid_id)
    return;

  auto dof_index = getDofIndex();

  auto soln = (*_sys.currentSolution())(dof_index);

  auto residual = soln - _value;

  for (auto tag_id : _vector_tags)
    if (_sys.hasVector(tag_id))
      _sys.getVector(tag_id).set(dof_index, residual);
}

void
FVPressurePin::computeJacobian()
{
  if (_node_id == libMesh::DofObject::invalid_id)
    return;

  auto dof_index = getDofIndex();

  Number jacobian = 1;

  // Cache the user's computeQpJacobian() value for later use.
  for (auto tag : _matrix_tags)
    if (_sys.hasMatrix(tag))
      _fe_problem.assembly(0).cacheJacobian(dof_index, dof_index, jacobian, tag);
}

void
FVPressurePin::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
}
