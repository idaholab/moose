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

#include "MFEMPMLKernel.h"

/**
 * PML-stretched curl-curl integrator: (c1 curl u, curl v) with c1 = detJ^{-1} J^T J scaled by the
 * base reluctivity coefficient.
 */
class MFEMPMLCurlCurlKernel : public MFEMPMLKernel
{
public:
  static InputParameters validParams();
  MFEMPMLCurlCurlKernel(const InputParameters & parameters);

protected:
  mfem::BilinearFormIntegrator * makeIntegrator(mfem::VectorCoefficient & coef) override
  {
    return new mfem::CurlCurlIntegrator(coef);
  }
};

#endif