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
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false),
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
  if (isParamValid("interface_boundaries"))
  {
    const auto & interface_boundaries = getParam<std::vector<BoundaryName>>("interface_boundaries");
    _interface_bnd_ids = MooseMeshUtils::getBoundaryIDSet(_mesh, interface_boundaries, false);

    for (const auto interface_bnd_id : _interface_bnd_ids)
    {
      // check on which side the interface is connected with the subdomain of this domain user
      // object
      const auto & primary_connected_blocks = _mesh.getBoundaryConnectedBlocks(interface_bnd_id);
      bool has_full_primary_connection = true;
      for (const auto interface_connected_block : primary_connected_blocks)
        if (!hasBlocks(interface_connected_block))
          has_full_primary_connection = false;

      const auto & secondary_connected_blocks =
          _mesh.getBoundaryConnectedSecondaryBlocks(interface_bnd_id);
      bool has_full_secondary_connection = true;
      for (const auto interface_connected_block : secondary_connected_blocks)
        if (!hasBlocks(interface_connected_block))
          has_full_secondary_connection = false;

      // we push the subdomains on the other side for the interface
      if (has_full_primary_connection)
        _interface_connected_blocks[interface_bnd_id] = secondary_connected_blocks;
      else if (has_full_secondary_connection)
        _interface_connected_blocks[interface_bnd_id] = primary_connected_blocks;
      else
        paramError("interface_boundaries",
                   "Not all sides in the interface with ID ",
                   interface_bnd_id,
                   " is connected with the domain of the domain user object ",
                   name());
    }
  }
}

const MooseVariableFieldBase *
DomainUserObject::getInterfaceFieldVar(const std::string & var_name,
                                       const unsigned int comp,
                                       const std::set<BoundaryID> * interfaces)
{
  const auto * const field_var = getFieldVar(var_name, comp);
  mooseAssert(field_var, "We should not be able to return a null variable");
  if (interfaces)
    _var_interfaces[field_var->name()] = *interfaces;
  else
    _var_interfaces[field_var->name()] = _interface_bnd_ids;
  return field_var;
}

void
DomainUserObject::checkVariable(const MooseVariableFieldBase & variable) const
{
  auto it = _var_interfaces.find(variable.name());
  if (it != _var_interfaces.end())
  {
    // we could have done this check in the constructor but to be consistent with other block
    // restrictable checks, we'll do it here
    auto & bnd_ids = it->second;
    for (const auto bnd_id : bnd_ids)
    {
      const auto & connected_blocks = libmesh_map_find(_interface_connected_blocks, bnd_id);
      for (const auto & bid : connected_blocks)
        if (!variable.hasBlocks(bid))
          mooseError("Variable '",
                     variable.name(),
                     "' is not defined on the interface connected block '",
                     _mesh.getSubdomainName(bid),
                     "'");
    }
  }
  else
    BlockRestrictable::checkVariable(variable);
}
