//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideUserObject.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "Assembly.h"
#include "FEProblemBase.h"
#include "MaterialBase.h"
#include "MaterialWarehouse.h"

InputParameters
SideUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += BoundaryRestrictableRequired::validParams();
  params += MaterialPropertyInterface::validParams();
  params.addRelationshipManager("GhostLowerDElems",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC);
  return params;
}

SideUserObject::SideUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    BoundaryRestrictableRequired(this, false), // false for applying to sidesets
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    TransientInterface(this),
    ElementIDInterface(this),
    _mesh(_subproblem.mesh()),
    _q_point(_assembly.qPointsFace()),
    _qrule(_assembly.qRuleFace()),
    _JxW(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation()),
    _normals(_assembly.normals()),
    _current_elem(_assembly.elem()),
    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),
    _current_boundary_id(_assembly.currentBoundaryID())
{
}

void
SideUserObject::initialSetup()
{
  UserObject::initialSetup();
  checkNoInterfaceMaterialPropertyDependencies();
}

void
SideUserObject::checkNoInterfaceMaterialPropertyDependencies() const
{
  const auto & consumed_props = getMatPropDependencies();
  if (consumed_props.empty())
    return;

  const auto & interface_materials = _fe_problem.getInterfaceMaterialsWarehouse();
  const auto & registry = _fe_problem.getMaterialPropertyRegistry();

  for (const auto bnd_id : boundaryIDs())
  {
    if (!interface_materials.hasActiveBoundaryObjects(bnd_id, _tid))
      continue;

    for (const auto & material : interface_materials.getActiveBoundaryObjects(bnd_id, _tid))
      for (const auto supplied_prop : material->getSuppliedPropIDs())
        if (consumed_props.count(supplied_prop))
          paramError("boundary",
                     "Side user object '",
                     name(),
                     "' consumes material property '",
                     registry.getName(supplied_prop),
                     "', which is declared by interface material '",
                     material->name(),
                     "'. Side user objects do not execute in an interface material context; use an "
                     "InterfaceUserObject or InterfacePostprocessor for interface quantities.");
  }
}

void
SideUserObject::getFaceInfos()
{
  _face_infos.clear();

  // Either the element or the (active) neighbor is a valid argument to get a face info
  const Elem * side_neighbor = _current_elem->neighbor_ptr(_current_side);

  mooseAssert(_current_elem, "We should have an element");
  mooseAssert(_current_elem->active(), "The current element should be active");

  // No neighbor means we are at a boundary, a FaceInfo exists in the mesh
  if (side_neighbor)
  {
    std::vector<const Elem *> candidate_neighbors = {side_neighbor};

    // neighbor is not active, we have to seek its refined children to get a FaceInfo
    if (!side_neighbor->active())
      side_neighbor->active_family_tree_by_neighbor(candidate_neighbors, _current_elem);

    for (const Elem * neighbor : candidate_neighbors)
    {
      const Elem * element = _current_elem;
      auto side = _current_side;

      // If a neighbor exists, the face info may only be defined on the other side
      // First check refinement level
      if (_current_elem->level() < neighbor->level())
      {
        element = neighbor;
        side = neighbor->which_neighbor_am_i(_current_elem);
      }
      // Then check ids
      else if ((_current_elem->level() == neighbor->level()) &&
               (_current_elem->id() > neighbor->id()))
      {
        element = neighbor;
        side = neighbor->which_neighbor_am_i(_current_elem);
      }
      const auto fi = _mesh.faceInfo(element, side);
      mooseAssert(fi, "Face info must not be null.");
      _face_infos.push_back(fi);
    }
  }
  else
  {
    const auto fi = _mesh.faceInfo(_current_elem, _current_side);
    mooseAssert(fi, "Face info must not be null.");
    _face_infos.push_back(fi);
  }
}
