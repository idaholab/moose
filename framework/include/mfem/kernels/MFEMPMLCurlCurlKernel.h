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
 * Perfectly matched layer curl curl integrator: (c1 curl u, curl v) with c1 = det(J)^-1 J^T J
 * scaled by the base reluctivity coefficient, where J is the Jacobian of the coordinate stretch.
 */
class MFEMPMLCurlCurlKernel : public MFEMPMLKernel
{
public:
  static InputParameters validParams();
  MFEMPMLCurlCurlKernel(const InputParameters & parameters);

protected:
  mfem::BilinearFormIntegrator *
  makeIntegrator(MFEMPMLMatrixCoefficient::ComplexComponent comp) override
  {
    // In two dimensions the curl of a vector field is a scalar, so this term picks up only the
    // inverse determinant of the stretch rather than the full tensor.
    if (_stretch_vec.dim() == 2)
      return new mfem::CurlCurlIntegrator(comp == MFEMPMLMatrixCoefficient::RE ? _scalar_re
                                                                               : _scalar_im);
    return new mfem::CurlCurlIntegrator(comp == MFEMPMLMatrixCoefficient::RE ? _matrix_re
                                                                             : _matrix_im);
  }
};

#endif
