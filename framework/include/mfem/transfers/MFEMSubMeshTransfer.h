//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

/**
 * Class to transfer MFEM variable data to or from a restricted copy of the variable defined on an a
 * subspace of an MFEMMesh, represented as an MFEMSubMesh.
 */
class MFEMSubMeshTransfer : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMSubMeshTransfer(const InputParameters & parameters);

  /// Executes the transfer.
  virtual void execute() override;

private:
  /// Name of source MFEMVariable to transfer DoF data from.
  const VariableName & _source_var_name;
  /// Reference to source gridfunction.
  const mfem::ParGridFunction & _source_var;
  /// Name of MFEMVariable to store the transferred output in.
  const VariableName & _result_var_name;
  /// Reference to result gridfunction.
  mfem::ParGridFunction & _result_var;
};

#endif
