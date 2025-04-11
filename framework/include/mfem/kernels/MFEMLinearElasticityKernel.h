#pragma once
#include "MFEMKernel.h"

/*
(cᵢₖⱼₗ∇uⱼ, ∇vᵢ),
cᵢₖⱼₗ = λ δᵢₖδⱼₗ + μ (δᵢⱼδₖₗ + δᵢₗδⱼₖ),
λ = (Eν)/((1-2ν)(1+ν)),
μ = E/(2(1+ν)),
E is Young's modulus,
ν is Poisson's ratio
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
