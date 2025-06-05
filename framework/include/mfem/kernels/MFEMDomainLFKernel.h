#ifdef MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

/*
(f, v)
*/
class MFEMDomainLFKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMDomainLFKernel(const InputParameters & parameters);
  ~MFEMDomainLFKernel() override {}

  virtual mfem::LinearFormIntegrator * createLFIntegrator() override;

protected:
  mfem::Coefficient & _coef;
};

#endif
