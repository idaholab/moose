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
 * Scalar perfectly matched layer coefficient a/det(J), where J is the Jacobian of the coordinate
 * stretch and a is a base scalar coefficient. In two dimensions the curl of a vector field is a
 * scalar, so the curl curl term picks up only the inverse determinant rather than a tensor.
 *
 * Only the real or imaginary part is returned, as the complex system is assembled from two real
 * bilinear forms.
 */
class MFEMPMLScalarCoefficient : public mfem::Coefficient
{
public:
  enum ComplexComponent
  {
    REAL,
    IMAGINARY
  };

  MFEMPMLScalarCoefficient(const MFEMPMLStretchVector & stretch,
                           mfem::Coefficient * const & base_coefficient,
                           ComplexComponent component)
    : _stretch_vec(stretch), _base_coefficient(base_coefficient), _component(component)
  {
  }

  mfem::real_t Eval(mfem::ElementTransformation & transformation,
                    const mfem::IntegrationPoint & integration_point) override;

private:
  const MFEMPMLStretchVector & _stretch_vec;
  /// Base scalar coefficient, such as the reluctivity. The function that declared this coefficient
  /// only looks it up once every coefficient of the problem has been declared, so what is held
  /// here is a reference to the pointer it fills in then.
  mfem::Coefficient * const & _base_coefficient;
  const ComplexComponent _component;
};

#endif
