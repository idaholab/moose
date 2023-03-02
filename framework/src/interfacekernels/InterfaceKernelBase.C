//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceKernelBase.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

InputParameters
InterfaceKernelBase::validParams()
{
  InputParameters params = NeighborResidualObject::validParams();
  params += BoundaryRestrictable::validParams();
  params += TwoMaterialPropertyInterface::validParams();

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation. Note that in "
                        "the case this is true but no displacements "
                        "are provided in the Mesh block the "
                        "undisplaced mesh will still be used.");
  params.addPrivateParam<bool>("_use_undisplaced_reference_points", false);
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.declareControllable("enable");
  params.addRequiredCoupledVar("neighbor_var", "The variable on the other side of the interface.");
  params.set<std::string>("_moose_base") = "InterfaceKernel";
  params.addParam<std::vector<AuxVariableName>>(
      "save_in",
      "The name of auxiliary variables to save this Kernel's residual contributions to. "
      " Everything about that variable must match everything about this variable (the "
      "type, what blocks it's on, etc.)");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in",
      "The name of auxiliary variables to save this Kernel's diagonal Jacobian "
      "contributions to. Everything about that variable must match everything "
      "about this variable (the type, what blocks it's on, etc.)");

  MultiMooseEnum save_in_var_side("m s");
  params.addParam<MultiMooseEnum>(
      "save_in_var_side",
      save_in_var_side,
      "This parameter must exist if save_in variables are specified and must have the same length "
      "as save_in. This vector specifies whether the corresponding aux_var should save-in "
      "residual contributions from the primary ('p') or secondary side ('s').");
  params.addParam<MultiMooseEnum>(
      "diag_save_in_var_side",
      save_in_var_side,
      "This parameter must exist if diag_save_in variables are specified and must have the same "
      "length as diag_save_in. This vector specifies whether the corresponding aux_var should "
      "save-in jacobian contributions from the primary ('p') or secondary side ('s').");
  params.addParamNamesToGroup("diag_save_in save_in save_in_var_side diag_save_in_var_side",
                              "Advanced");

  // InterfaceKernels always need one layer of ghosting.
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC |
                                    Moose::RelationshipManagerType::COUPLING);
  return params;
}

// Static mutex definitions
Threads::spin_mutex InterfaceKernelBase::_resid_vars_mutex;
Threads::spin_mutex InterfaceKernelBase::_jacoby_vars_mutex;

InterfaceKernelBase::InterfaceKernelBase(const InputParameters & parameters)
  : NeighborResidualObject(parameters),
    BoundaryRestrictable(this, false), // false for _not_ nodal
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false),
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
    ElementIDInterface(this),
    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),
    _neighbor_elem(_assembly.neighbor()),
    _neighbor_elem_volume(_assembly.neighborVolume()),
    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),
    _coord_sys(_assembly.coordSystem()),
    _q_point(_assembly.qPointsFace()),
    _qrule(_assembly.qRuleFace()),
    _JxW(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation()),
    _save_in_var_side(parameters.get<MultiMooseEnum>("save_in_var_side")),
    _save_in_strings(parameters.get<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in_var_side(parameters.get<MultiMooseEnum>("diag_save_in_var_side")),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName>>("diag_save_in"))

{
}

const Real &
InterfaceKernelBase::getNeighborElemVolume()
{
  return _assembly.neighborVolume();
}

void
InterfaceKernelBase::prepareShapes(const unsigned int var_num)
{
  _subproblem.prepareFaceShapes(var_num, _tid);
}
