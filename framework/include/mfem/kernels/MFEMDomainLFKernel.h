#ifdef MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

/*
 * \f[
 * (f, u')
 * \f]
 */
class MFEMDomainLFKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMDomainLFKernel(const InputParameters & parameters);

  virtual mfem::LinearFormIntegrator * createLFIntegrator() override;

protected:
  const std::string & _coef_name;
  mfem::Coefficient & _coef;
};

#endif
