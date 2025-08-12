#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMAuxKernel.h"

/**
 * Projects a vector coefficient onto a vector-valued aux variable.
 */
class MFEMVectorProjectAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMVectorProjectAux(const InputParameters & parameters);

  virtual ~MFEMVectorProjectAux() = default;

  virtual void execute() override;

protected:
  /// Reference to source coefficient.
  mfem::VectorCoefficient & _vec_coef;
};

#endif
