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
 * Perfectly matched layer vector FE mass integrator: (c2 u, v) with c2 = det(J) (J^T J)^-1 scaled
 * by the base mass coefficient, where J is the Jacobian of the coordinate stretch.
 */
class MFEMPMLVectorFEMassKernel : public MFEMPMLKernel
{
public:
  static InputParameters validParams();
  MFEMPMLVectorFEMassKernel(const InputParameters & parameters);

protected:
  mfem::BilinearFormIntegrator * makeIntegrator(MFEMPMLMatrixCoefficient::Part part) override
  {
    return new mfem::VectorFEMassIntegrator(part == MFEMPMLMatrixCoefficient::RE ? _matrix_re
                                                                                 : _matrix_im);
  }
};

#endif
