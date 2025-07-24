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
#include "MFEMMixedBilinearFormKernel.h"

/**
 * \f[
 * (k \vec \nabla \cdot \vec u, v)
 * \f]
 */
class MFEMVectorFEDivergenceKernel : public MFEMMixedBilinearFormKernel
{
public:
  static InputParameters validParams();

  MFEMVectorFEDivergenceKernel(const InputParameters & parameters);
  ~MFEMVectorFEDivergenceKernel() override = default;

  virtual mfem::BilinearFormIntegrator * createMBFIntegrator() override;

protected:
  mfem::Coefficient & _coef;
};

#endif
