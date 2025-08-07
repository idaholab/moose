#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMAuxKernel.h"
#include "mfem.hpp"

/**
 * Projects a vector Coefficient into a vector-valued aux variable.
 */
class MFEMVectorProjectAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMVectorProjectAux(const InputParameters & parameters);

  virtual ~MFEMVectorProjectAux() = default;

  virtual void execute() override;

protected:
  mfem::VectorCoefficient & _vec_coef;
};

#endif
