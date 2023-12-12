//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalPatchRecoveryAux.h"
#include "NodalPatchRecoveryBase.h"

registerMooseObject("MooseApp", NodalPatchRecoveryAux);

InputParameters
NodalPatchRecoveryAux::validParams()
{
  InputParameters params = NodalPatchRecoveryAuxBase::validParams();
  return params;
}

NodalPatchRecoveryAux::NodalPatchRecoveryAux(const InputParameters & parameters)
  : NodalPatchRecoveryAuxBase(parameters),
    _npr(getUserObject<NodalPatchRecoveryBase>("nodal_patch_recovery_uo"))
{
  // Check user object block restriction for consistency
  if (!isBlockSubset(_npr.blockIDs()))
    paramError("nodal_patch_recovery_uo",
               "Nodal patch recovery auxiliary kernel is not defined in a subset of blocks of the "
               "associated user object. Revise your input file.");
}
Real
NodalPatchRecoveryAux::nodalPatchRecovery()
{
  return _npr.nodalPatchRecovery(*_current_node, _elem_ids);
}
