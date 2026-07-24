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
 * PML-stretched vector FE mass integrator: (c2 u, v) with c2 = detJ (J^T J)^{-1} scaled by the
 * base mass coefficient, where J is the Jacobian of the complex PML coordinate stretch.
 */
class MFEMPMLVectorFEMassKernel : public MFEMPMLKernel
{
public:
  static InputParameters validParams();
  MFEMPMLVectorFEMassKernel(const InputParameters & parameters);

protected:
  mfem::BilinearFormIntegrator * makeIntegrator(mfem::VectorCoefficient & coef) override
  {
    return new mfem::VectorFEMassIntegrator(coef);
  }
};

#endif
