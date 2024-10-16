#pragma once
#include "MFEMEssentialBC.h"
#include "MFEMFunctionCoefficient.h"

class MFEMScalarDirichletBC : public MFEMEssentialBC
{
public:
  static InputParameters validParams();

  MFEMScalarDirichletBC(const InputParameters & parameters);

  void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) override;

protected:
  MFEMCoefficient * _coef{nullptr};
};
