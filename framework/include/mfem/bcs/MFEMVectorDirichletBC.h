#ifdef MFEM_ENABLED

#pragma once

#include "MFEMVectorDirichletBCBase.h"

class MFEMVectorDirichletBC : public MFEMVectorDirichletBCBase
{
public:
  static InputParameters validParams();
  MFEMVectorDirichletBC(const InputParameters & parameters);
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh) override;
};

#endif
