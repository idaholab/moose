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
 * - (k \vec u, \vec \nabla v)
 * \f]
 */
class MFEMVectorFEWeakDivergenceKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMVectorFEWeakDivergenceKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;

protected:
  mfem::Coefficient & _coef;
};

#endif
