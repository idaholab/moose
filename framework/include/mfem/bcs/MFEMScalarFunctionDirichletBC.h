#ifdef MFEM_ENABLED

#pragma once
#include "MFEMEssentialBC.h"

class MFEMScalarFunctionDirichletBC : public MFEMEssentialBC
{
public:
  static InputParameters validParams();

  MFEMScalarFunctionDirichletBC(const InputParameters & parameters);

  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh) override;

protected:
  std::shared_ptr<mfem::FunctionCoefficient> _coef{nullptr};
};

#endif
