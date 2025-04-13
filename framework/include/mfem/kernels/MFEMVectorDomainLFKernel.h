#ifdef MFEM_ENABLED

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

  virtual mfem::LinearFormIntegrator * createIntegrator() override;

protected:
  const std::string _vec_coef_name;
  mfem::VectorCoefficient & _vec_coef;
};

#endif
