#ifdef MFEM_ENABLED

#pragma once

#include "MFEMVectorFunctionDirichletBCBase.h"

class MFEMVectorFunctionNormalDirichletBC : public MFEMVectorFunctionDirichletBCBase
{
public:
  static InputParameters validParams();
  MFEMVectorFunctionNormalDirichletBC(const InputParameters & parameters);
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh) override;
};

#endif
