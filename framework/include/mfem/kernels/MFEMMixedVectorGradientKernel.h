#pragma once
#include "MFEMKernel.h"

/*
(σ ∇ V, u')
*/
class MFEMMixedVectorGradientKernel : public MFEMKernel<mfem::BilinearFormIntegrator>
{
public:
  static InputParameters validParams();

  MFEMMixedVectorGradientKernel(const InputParameters & parameters);
  ~MFEMMixedVectorGradientKernel() override = default;

  virtual mfem::BilinearFormIntegrator * createIntegrator() override;

protected:
  std::string _coef_name;
  mfem::Coefficient & _coef;
};
