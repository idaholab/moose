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

defineLegacyParams(InternalSideUserObject);

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
    _current_neighbor_volume(_assembly.neighborVolume())
{
  // Keep track of which variables are coupled so we know what we depend on
  const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);
}

const Real &
InternalSideUserObject::getNeighborElemVolume()
{
  return _assembly.neighborVolume();
}
