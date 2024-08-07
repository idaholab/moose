#pragma once
#include "MFEMBilinearFormKernel.h"
#include "kernels.h"

/*
(σ ∇ q, ∇ q')
*/
class MFEMDiffusionKernel : public MFEMBilinearFormKernel
{
public:
  static InputParameters validParams();

  MFEMDiffusionKernel(const InputParameters & parameters);
  ~MFEMDiffusionKernel() override {}

  virtual mfem::BilinearFormIntegrator * createIntegrator() override;

protected:
  std::string _coef_name;
  mfem::Coefficient * _coef{nullptr};
};
