//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

defineLegacyParams(SideUserObject);

InputParameters
SideUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += BoundaryRestrictableRequired::validParams();
  params += MaterialPropertyInterface::validParams();
  return params;
}

SideUserObject::SideUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    BoundaryRestrictableRequired(this, false), // false for applying to sidesets
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
    Coupleable(this, false),
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
    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),
    _current_boundary_id(_assembly.currentBoundaryID())
{
  // Keep track of which variables are coupled so we know what we depend on
  const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);
}
