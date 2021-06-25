//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceUserObject.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "Assembly.h"

#include "libmesh/remote_elem.h"

InputParameters
InterfaceUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += BoundaryRestrictableRequired::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params += TransientInterface::validParams();
  params.addClassDescription("Basic UO class to perform computation across an interface");

  // Need one layer of ghosting
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC);
  return params;
}

InterfaceUserObject::InterfaceUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    BoundaryRestrictableRequired(this, false), // false for applying to sidesets
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
    NeighborCoupleable(this, false, false),
    MooseVariableDependencyInterface(),
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
    _current_neighbor_volume(_assembly.neighborVolume()),
    _fi(nullptr)
{
  // Keep track of which variables are coupled so we know what we depend on
  const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);

  // Check for finite volume variables
  _has_fv_vars = false;
  for (const auto & var : coupled_vars)
    if (var->isFV())
      _has_fv_vars = true;
}

void
InterfaceUserObject::execute()
{
  if (_has_fv_vars)
  {
    // Retrieve the face info from the mesh
    _fi = _mesh.faceInfo(_current_elem, _current_side);
    if (!_fi)
    {
      // Let's check the other side
      const Elem * const neighbor = _current_elem->neighbor_ptr(_current_side);
      mooseAssert(neighbor != remote_elem,
                  "I'm pretty confident that if we got here then our neighbor should be "
                  "local/ghosted/null");
      if (neighbor)
      {
        const auto neigh_side = neighbor->which_neighbor_am_i(_current_elem);
        _fi = _mesh.faceInfo(neighbor, neigh_side);
      }

      if (!_fi)
        // We still don't have a face info. It must be owned by another process
        return;
    }

    auto pr = _face_infos_processed.insert(_fi);
    if (!pr.second)
      // Insertion didn't happen so we must have already processed this FaceInfo
      return;
  }
}

void
InterfaceUserObject::initialize()
{
  if (_has_fv_vars)
    _face_infos_processed.clear();
}

const Real &
InterfaceUserObject::getNeighborElemVolume()
{
  return _current_neighbor_volume;
}
