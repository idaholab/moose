//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMFlattenedMatrixCoefficient.h"
#include "libmesh/int_range.h"

MFEMFlattenedMatrixCoefficient::MFEMFlattenedMatrixCoefficient(mfem::MatrixCoefficient & mat_coef)
  : mfem::VectorCoefficient(mat_coef.GetHeight() * mat_coef.GetWidth()), _mat_coef(&mat_coef)
{
}

void
MFEMFlattenedMatrixCoefficient::SetTime(mfem::real_t t)
{
  _mat_coef->SetTime(t);
  this->mfem::VectorCoefficient::SetTime(t);
}

void
MFEMFlattenedMatrixCoefficient::Eval(mfem::Vector & V,
                                     mfem::ElementTransformation & T,
                                     const mfem::IntegrationPoint & ip)
{
  _mat_coef->Eval(_mat, T, ip);
  const int width = _mat.Width();
  V.SetSize(vdim);
  for (const auto i : make_range(_mat.Height()))
    for (const auto j : make_range(width))
      V(i * width + j) = _mat(i, j);
}

#endif
