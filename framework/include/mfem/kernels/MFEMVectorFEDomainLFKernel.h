#ifdef MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

/*
(\\vec f, \\vec u')
*/
class MFEMVectorFEDomainLFKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMVectorFEDomainLFKernel(const InputParameters & parameters);

  virtual mfem::LinearFormIntegrator * createLFIntegrator() override;

protected:
  mfem::VectorCoefficient & _vec_coef;
};

#endif
