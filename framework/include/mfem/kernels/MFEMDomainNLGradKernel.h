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
 * (f, Grad(v))
 * \f]
 */
class MFEMDomainNLGradKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMDomainNLGradKernel(const InputParameters & parameters);

  virtual mfem::LinearFormIntegrator *  createNLAIntegrator() override;

protected:
  mfem::VectorCoefficient & _coef;
};

#endif
