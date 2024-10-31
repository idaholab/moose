#pragma once
#include "MFEMEssentialBC.h"

class MFEMVectorFunctionDirichletBC : public MFEMEssentialBC
{
protected:
  std::shared_ptr<mfem::VectorFunctionCoefficient> _vec_coef{nullptr};
  enum APPLY_TYPE
  {
    STANDARD,
    TANGENTIAL,
    NORMAL
  };

public:
  static InputParameters validParams();

  MFEMVectorFunctionDirichletBC(const InputParameters & parameters);

  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) override;

  APPLY_TYPE _boundary_apply_type;
};
