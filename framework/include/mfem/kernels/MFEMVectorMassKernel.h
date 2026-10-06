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
 * (k \vec u, \vec v)
 * \f]
 * for \f$\vec u\f$ and \f$\vec v\f$ in vector-valued \f$H^1\f$ or \f$L^2\f$ spaces, whose
 * components are each discretised with a scalar basis.
 */
class MFEMVectorMassKernel : public MFEMMixedBilinearFormKernel
{
public:
  static InputParameters validParams();

  MFEMVectorMassKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createMBFIntegrator() override;

protected:
  mfem::Coefficient & _coef;
};

#endif
