#ifdef MFEM_ENABLED

#pragma once
#include "MFEMEssentialBC.h"

class MFEMScalarDirichletBC : public MFEMEssentialBC
{
public:
  static InputParameters validParams();

  MFEMScalarDirichletBC(const InputParameters & parameters);

  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh) override;

protected:
  mfem::ConstantCoefficient & _coef;
};

#endif
