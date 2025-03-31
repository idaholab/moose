#ifdef MFEM_ENABLED

#pragma once
#include "MFEMEssentialBC.h"

class MFEMScalarFunctorDirichletBC : public MFEMEssentialBC
{
public:
  static InputParameters validParams();

  MFEMScalarFunctorDirichletBC(const InputParameters & parameters);

  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh) override;

protected:
  std::string _coef_name;
  mfem::Coefficient & _coef;
};

#endif
