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

// all this class needs to do differently is to implement createLFIntegrator
class MFEMDGDirichletLFKernel : public MFEMKernel
{
public:
  static InputParameters validParams();
  MFEMDGDirichletLFKernel(const InputParameters & parameters);
  virtual mfem::LinearFormIntegrator * createFaceLFIntegrator() override;

protected:
  /// Name of (the test variable associated with) the weak form that the kernel is applied to.
  int _fe_order;
  mfem::ConstantCoefficient _one;
  mfem::ConstantCoefficient _zero;
  mfem::real_t _sigma;
  mfem::real_t _kappa;
};

#endif
