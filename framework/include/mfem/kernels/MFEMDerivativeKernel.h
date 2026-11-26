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
 * (k \partial_i u, v)
 * \f]
 */
class MFEMDerivativeKernel : public MFEMMixedBilinearFormKernel
{
public:
  static InputParameters validParams();

  MFEMDerivativeKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createMBFIntegrator() override;

protected:
  mfem::Coefficient & _coef;
  /// Index i of the spatial direction the trial variable is differentiated along
  const unsigned int _component;
};

#endif
