#ifdef MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

/*
(\\vec f, \\vec u')
*/
class MFEMVectorDomainLFKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMVectorDomainLFKernel(const InputParameters & parameters);

  virtual mfem::LinearFormIntegrator * createLFIntegrator() override;

protected:
  const MFEMVectorCoefficientName & _vec_coef_name;
  mfem::VectorCoefficient & _vec_coef;
};

#endif
