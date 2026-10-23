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
 * \sum_e \eta (r_e([u]), r_e([v]))
 * \f]
 */
class MFEMDGDiffusionBR2Kernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMDGDiffusionBR2Kernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;

  virtual bool isDGKernel() const override { return true; }

protected:
  /// DG space of the variable, needed by MFEM to build the lifting operators
  mfem::ParFiniteElementSpace & _fespace;
  mfem::Coefficient & _coef;
  mfem::real_t _eta;
};

#endif
