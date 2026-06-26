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

// This class needs to implement createFaceBFIntegrator
class MFEMDGDiffusionKernel : public MFEMKernel
{
public:
  static InputParameters validParams();
  MFEMDGDiffusionKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createFaceBFIntegrator() override;

protected:
  int _fe_order;
  mfem::ConstantCoefficient _one;
  mfem::real_t _sigma;
  mfem::real_t _kappa;
};

#endif
