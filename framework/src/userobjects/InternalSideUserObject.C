//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalSideUserObject.h"
#include "Assembly.h"

InputParameters
InternalSideUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += BlockRestrictable::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params += TransientInterface::validParams();

  // Need one layer of ghosting
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC);

  return params;
}

InternalSideUserObject::InternalSideUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    BlockRestrictable(this),
    TwoMaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false),
    TransientInterface(this),
    ElementIDInterface(this),
    _mesh(_subproblem.mesh()),
    _q_point(_assembly.qPointsFace()),
    _qrule(_assembly.qRuleFace()),
    _JxW(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation()),
    _normals(_assembly.normals()),
    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),
    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),
    _neighbor_elem(_assembly.neighbor()),
    _current_neighbor_volume(_assembly.neighborVolume())
{
}

const Real &
InternalSideUserObject::getNeighborElemVolume()
{
  return _assembly.neighborVolume();
}

void
InternalSideUserObject::getFaceInfos()
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
