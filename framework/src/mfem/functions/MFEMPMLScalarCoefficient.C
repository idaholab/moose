//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPMLScalarCoefficient.h"

mfem::real_t
MFEMPMLScalarCoefficient::Eval(mfem::ElementTransformation & transformation,
                               const mfem::IntegrationPoint & integration_point)
{
  transformation.SetIntPoint(&integration_point);

  const std::complex<mfem::real_t> value =
      _base_coefficient->Eval(transformation, integration_point) /
      _stretch_vec.jacobian(transformation).determinant();

  return (_component == REAL) ? value.real() : value.imag();
}

#endif
