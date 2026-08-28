//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPMLMatrixCoefficient.h"

void
MFEMPMLMatrixCoefficient::Eval(mfem::DenseMatrix & K,
                               mfem::ElementTransformation & transformation,
                               const mfem::IntegrationPoint & integration_point)
{
  using ComplexMatrix = MFEMPMLStretchVector::ComplexMatrix;

  const int dim = _stretch_vec.dim();
  transformation.SetIntPoint(&integration_point);

  const ComplexMatrix jacobian = _stretch_vec.jacobian(transformation);
  const ComplexMatrix product = jacobian.transpose() * jacobian;
  const std::complex<mfem::real_t> determinant = jacobian.determinant();
  const ComplexMatrix tensor = (_tensor == CURL)
                                   ? ComplexMatrix(product / determinant)
                                   : ComplexMatrix(product.inverse() * determinant);

  const mfem::real_t base = _base_coefficient->Eval(transformation, integration_point);

  K.SetSize(dim);
  for (const auto a : make_range(dim))
    for (const auto b : make_range(dim))
      K(a, b) = base * ((_component == REAL) ? tensor(a, b).real() : tensor(a, b).imag());
}

#endif