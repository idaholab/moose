#ifdef MFEM_ENABLED

#pragma once

#include "MFEMVectorDirichletBCBase.h"

class MFEMVectorNormalDirichletBC : public MFEMVectorDirichletBCBase
{
public:
  static InputParameters validParams();
  MFEMVectorNormalDirichletBC(const InputParameters & parameters);
  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh) override;
};

#endif
