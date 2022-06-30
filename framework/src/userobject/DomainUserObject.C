//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DomainUserObject.h"
#include "MooseVariableFE.h"
#include "SubProblem.h"
#include "Assembly.h"

InputParameters
DomainUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += BlockRestrictable::validParams();
  params += ThreeMaterialPropertyInterface::validParams();
  params += TransientInterface::validParams();
  params += RandomInterface::validParams();
  params.addParam<std::vector<BoundaryName>>(
      "interface_boundaries", "The interface boundaries on which this object will execute");
  // Need one layer of ghosting
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC);
  return params;
}

DomainUserObject::DomainUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    BlockRestrictable(this),
    ThreeMaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS),
    NeighborCoupleable(this, false, false),
    MooseVariableDependencyInterface(this),
    TransientInterface(this),
    RandomInterface(parameters, _fe_problem, _tid, false),
    ElementIDInterface(this),
    _mesh(_subproblem.mesh()),
    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),
    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),
    _neighbor_elem(_assembly.neighbor()),
    _current_neighbor_volume(_assembly.neighborVolume()),
    _current_boundary_id(_assembly.currentBoundaryID()),
    _normals(_assembly.normals()),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _q_point_face(_assembly.qPointsFace()),
    _qrule_face(_assembly.qRuleFace()),
    _JxW_face(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation())
{
  // Keep track of which variables are coupled so we know what we depend on
  const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);

  if (isParamValid("interface_boundaries"))
  {
    const auto & interface_boundaries = getParam<std::vector<BoundaryName>>("interface_boundaries");
    const auto & interface_bnd_ids_vec = _mesh.getBoundaryIDs(interface_boundaries);
    _interface_bnd_ids =
        std::set<BoundaryID>(interface_bnd_ids_vec.begin(), interface_bnd_ids_vec.end());

    for (const auto interface_bnd_id : _interface_bnd_ids)
    {
      const auto & interface_connected_blocks = _mesh.getInterfaceConnectedBlocks(interface_bnd_id);
      for (const auto interface_connected_block : interface_connected_blocks)
      {
        if (hasBlocks(interface_connected_block))
          // we're operating on this block
          continue;

        // these are blocks connected to our blocks
        _interface_connected_blocks.insert(interface_connected_block);
      }
    }
  }
}

const MooseVariableFieldBase *
DomainUserObject::getInterfaceFieldVar(const std::string & var_name, const unsigned int comp)
{
  const auto * const field_var = getFieldVar(var_name, comp);
  mooseAssert(field_var, "We should not be able to return a null variable");
  _interface_vars.insert(field_var);
  return field_var;
}

void
DomainUserObject::checkVariable(const MooseVariableFieldBase & variable) const
{
  if (_interface_vars.count(&variable))
  {
    // we could have done this check in the constructor but to be consistent with other block
    // restrictable checks, we'll do it here
    for (const auto connected_block : _interface_connected_blocks)
      if (!variable.hasBlocks(connected_block))
        mooseError("Variable '",
                   variable.name(),
                   "' is not defined on the interface connected block '",
                   _mesh.getSubdomainName(connected_block),
                   "'");
  }
  else
    BlockRestrictable::checkVariable(variable);
}
