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

#include "MFEMPMLStretchVector.h"

/**
 * Perfectly matched layer tensor coefficient.
 *
 * Which tensor applies is set by the quantity the bilinear form integrates, not by the operator
 * using it: pulling the weak form back from stretched to physical coordinates gives one factor for
 * an integrand holding the curl of the field and another for one holding the field itself.
 *
 *     CURL  = det(J)^-1 J^T J
 *     FIELD = det(J) (J^T J)^-1
 *
 * The stretch Jacobian J is a full matrix, since the level sets of the harmonic coordinate bend
 * the stretch direction from point to point. It is complex symmetric rather than Hermitian, so
 * both tensors are formed with the plain transpose.
 *
 * The result is scaled by a base scalar coefficient, and only its real or imaginary part is
 * returned, as the complex system is assembled from two real bilinear forms.
 */
class MFEMPMLMatrixCoefficient : public mfem::MatrixCoefficient
{
public:
  enum TensorType
  {
    CURL,
    FIELD
  };
  enum ComplexComponent
  {
    REAL,
    IMAGINARY
  };

  MFEMPMLMatrixCoefficient(const MFEMPMLStretchVector & stretch,
                           mfem::Coefficient * const & base_coefficient,
                           TensorType tensor,
                           ComplexComponent component)
    : mfem::MatrixCoefficient(stretch.dim()),
      _stretch_vec(stretch),
      _base_coefficient(base_coefficient),
      _tensor(tensor),
      _component(component)
  {
  }

  /// Which of the two tensors above this coefficient evaluates.
  TensorType tensorType() const { return _tensor; }

  void Eval(mfem::DenseMatrix & K,
            mfem::ElementTransformation & transformation,
            const mfem::IntegrationPoint & integration_point) override;

private:
  const MFEMPMLStretchVector & _stretch_vec;
  /// Base scalar coefficient, such as the reluctivity or the mass coefficient. The function that
  /// declared this coefficient only looks it up once every coefficient of the problem has been
  /// declared, so what is held here is a reference to the pointer it fills in then.
  mfem::Coefficient * const & _base_coefficient;
  const TensorType _tensor;
  const ComplexComponent _component;
};

#endif
