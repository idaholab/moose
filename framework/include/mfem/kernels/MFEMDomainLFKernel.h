#ifdef MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

/*
(f, u')
*/
class MFEMDomainLFKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMDomainLFKernel(const InputParameters & parameters);

  virtual mfem::LinearFormIntegrator * createLFIntegrator() override;

protected:
  const std::string _coef_name;
  mfem::Coefficient & _coef;
};

#endif
