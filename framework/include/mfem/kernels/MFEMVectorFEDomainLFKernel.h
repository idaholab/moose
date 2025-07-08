//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

/**
 * \f[
 * (\vec f, \vec v)
 * \f]
 */
class MFEMVectorFEDomainLFKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMVectorFEDomainLFKernel(const InputParameters & parameters);

  virtual std::pair<mfem::LinearFormIntegrator *, mfem::LinearFormIntegrator *> createLFIntegrator() override;

protected:
  mfem::VectorCoefficient & _vec_coef;

  const MFEMVectorCoefficientName & _vec_coef_imag_name;
  mfem::VectorCoefficient & _vec_coef_imag;
};

#endif
