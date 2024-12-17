#pragma once
#include "MFEMEssentialBC.h"

class MFEMScalarDirichletBC : public MFEMEssentialBC
{
public:
  static InputParameters validParams();

  MFEMScalarDirichletBC(const InputParameters & parameters);

  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) override;

protected:
  std::shared_ptr<mfem::ConstantCoefficient> _coef{nullptr};
};
