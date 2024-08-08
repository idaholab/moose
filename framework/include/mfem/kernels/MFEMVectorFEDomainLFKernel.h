#pragma once
#include "MFEMKernel.h"
#include "kernels.h"

/*
(\\vec f, \\vec u')
*/
class MFEMVectorFEDomainLFKernel : public MFEMKernel<mfem::LinearFormIntegrator>
{
public:
  static InputParameters validParams();

  MFEMVectorFEDomainLFKernel(const InputParameters & parameters);
  ~MFEMVectorFEDomainLFKernel() override {}

  virtual mfem::LinearFormIntegrator * createIntegrator() override;

protected:
  std::string _vec_coef_name;
  mfem::VectorCoefficient * _vec_coef{nullptr};
};
