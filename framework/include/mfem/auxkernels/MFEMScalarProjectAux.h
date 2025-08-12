#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMAuxKernel.h"

/**
 * Projects a scalar coefficient onto a scalar-valued aux variable.
 */
class MFEMScalarProjectAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMScalarProjectAux(const InputParameters & parameters);

  virtual ~MFEMScalarProjectAux() = default;

  virtual void execute() override;

protected:
  /// Reference to source coefficient.
  mfem::Coefficient & _coef;
};

#endif
