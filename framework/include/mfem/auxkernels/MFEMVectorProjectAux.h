#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMAuxKernel.h"
#include "mfem.hpp"

class MFEMVectorProjectAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMVectorProjectAux(const InputParameters & parameters);

  virtual ~MFEMVectorProjectAux() = default;

  virtual void execute() override;

protected:
  const MFEMVectorCoefficientName _coefficient_name;
  mfem::VectorCoefficient * _vec_coef;
};

#endif
