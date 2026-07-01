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

class MFEMDGDiffusionKernel : public MFEMKernel
{
public:
  static InputParameters validParams();
  MFEMDGDiffusionKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;

  virtual bool IsDGKernel() const override { return true; }

protected:
  int _fe_order;
  mfem::ConstantCoefficient _coef;
  mfem::real_t _sigma;
  mfem::real_t _kappa;
};

#endif
