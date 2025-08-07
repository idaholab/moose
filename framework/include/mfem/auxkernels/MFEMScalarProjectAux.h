#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMAuxKernel.h"
#include "mfem.hpp"

/**
 * Projects a scalar Coefficient into a scalar-valued aux variable.
 */
class MFEMScalarProjectAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMScalarProjectAux(const InputParameters & parameters);

  virtual ~MFEMScalarProjectAux() = default;

  virtual void execute() override;

protected:
  mfem::Coefficient & _coef;
};

#endif
