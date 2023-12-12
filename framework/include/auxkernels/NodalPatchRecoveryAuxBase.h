//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class NodalPatchRecoveryBase;

class NodalPatchRecoveryAuxBase : public AuxKernel
{
public:
  static InputParameters validParams();

  NodalPatchRecoveryAuxBase(const InputParameters & parameters);

  /**
   * Block restrict elements on which to perform the variable/property nodal recovery.
   */
  void blockRestrictElements(std::vector<dof_id_type> & elem_ids,
                             const std::vector<dof_id_type> & node_to_elem_pair_elems) const;

protected:
  virtual Real computeValue() override;

  /// Override this to get the fitted value from a Nodal Patch Recovery User Object
  virtual Real nodalPatchRecovery() = 0;

  /// local patch of elements used for recovery
  std::vector<dof_id_type> _elem_ids;
};
