//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProjectedMaterialPropertyNodalPatchRecoveryAux.h"
#include "ProjectedStatefulMaterialNodalPatchRecovery.h"

registerMooseObject("MooseApp", ProjectedMaterialPropertyNodalPatchRecoveryAux);

InputParameters
ProjectedMaterialPropertyNodalPatchRecoveryAux::validParams()
{
  InputParameters params = NodalPatchRecoveryAuxBase::validParams();
  params.addRequiredParam<unsigned int>("component",
                                        "Serial access component for the recovered type");
  return params;
}

ProjectedMaterialPropertyNodalPatchRecoveryAux::ProjectedMaterialPropertyNodalPatchRecoveryAux(
    const InputParameters & parameters)
  : NodalPatchRecoveryAuxBase(parameters),
    _npr(getUserObject<ProjectedStatefulMaterialNodalPatchRecoveryBase>("nodal_patch_recovery_uo")),
    _component(getParam<unsigned int>("component"))
{
  // Check user object block restriction for consistency
  if (!isBlockSubset(_npr.blockIDs()))
    paramError("nodal_patch_recovery_uo",
               "Nodal patch recovery auxiliary kernel is not defined in a subset of blocks of the "
               "associated user object. Revise your input file.");
}

Real
ProjectedMaterialPropertyNodalPatchRecoveryAux::nodalPatchRecovery()
{
  return _npr.nodalPatchRecovery(*_current_node, _elem_ids, _component);
}
