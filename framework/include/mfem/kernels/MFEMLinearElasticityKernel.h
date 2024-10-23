#pragma once
#include "MFEMKernel.h"

/*
(λ ∇.u, ∇.u') + (μ (∇u + (∇u)ᵀ), ∇u' + (∇u')ᵀ)
*/
class MFEMLinearElasticityKernel : public MFEMKernel<mfem::BilinearFormIntegrator>
{
public:
  static InputParameters validParams();

  MFEMLinearElasticityKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createIntegrator() override;

protected:
  std::string _lambda_name;
  std::string _mu_name;
  mfem::Coefficient & _lambda;
  mfem::Coefficient & _mu;
};
