#pragma once
#include "MFEMKernel.h"

/*
(\\vec f, \\vec u')
*/
class MFEMVectorDomainLFKernel : public MFEMKernel<mfem::LinearFormIntegrator>
{
public:
  static InputParameters validParams();

  MFEMVectorDomainLFKernel(const InputParameters & parameters);
  ~MFEMVectorDomainLFKernel() override {}

  virtual mfem::LinearFormIntegrator * createIntegrator() override;

protected:
  std::shared_ptr<mfem::VectorCoefficient> _vec_coef{nullptr};
};
