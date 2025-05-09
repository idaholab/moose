#ifdef MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

/**
 * Constructs and stores an mfem::ParGridFunction object.
 */
class MFEMSubMeshTransfer : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMSubMeshTransfer(const InputParameters & parameters);

  // Executes the transfer.
  virtual void execute() override;

private:
  // Name of source MFEMVariable to take the curl of.
  const VariableName _source_var_name;
  // Reference to source gridfunction.
  const mfem::ParGridFunction & _source_var;
  /// Name of MFEMVariable to store the transferred output in.
  const VariableName _result_var_name;
  /// Reference to result gridfunction.
  mfem::ParGridFunction & _result_var;
};

#endif
