//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceUserObjectBase.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "Assembly.h"

#include "libmesh/remote_elem.h"

InputParameters
InterfaceUserObjectBase::validParams()
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

InterfaceUserObjectBase::InterfaceUserObjectBase(const InputParameters & parameters)
  : UserObject(parameters),
    BoundaryRestrictableRequired(this, false), // false for applying to sidesets
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
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
