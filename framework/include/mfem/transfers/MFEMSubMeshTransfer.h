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

#include "MFEMExecutedObject.h"

namespace Moose::MFEM
{
/**
 * Class to transfer MFEM variable data to or from a restricted copy of the variable defined on an a
 * subspace of an Moose::MFEM::Mesh, represented as an Moose::MFEM::SubMesh.
 */
class SubMeshTransfer : public ExecutedObject
{
public:
  static InputParameters validParams();

  SubMeshTransfer(const InputParameters & parameters);

  virtual std::optional<std::string> suppliedVariableName() const override;

  /// Executes the transfer.
  virtual void execute() override;

private:
  /// Name of source Moose::MFEM::Variable to transfer DoF data from.
  const VariableName & _source_var_name;
  /// Reference to source gridfunction.
  const mfem::ParGridFunction & _source_var;
  /// Name of Moose::MFEM::Variable to store the transferred output in.
  const VariableName & _result_var_name;
  /// Reference to result gridfunction.
  mfem::ParGridFunction & _result_var;
};

} // namespace Moose::MFEM
#endif
