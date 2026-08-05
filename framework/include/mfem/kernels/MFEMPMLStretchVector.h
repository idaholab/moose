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

#include "MFEMKernel.h"

/**
 * Perfectly matched layer coordinate stretch built on a harmonic coordinate.
 *
 * A scalar field psi solves Laplace's equation inside the layer, held at zero on its inner surface
 * and one on its outer surface, with no flux through any lateral wall. Its level sets foliate the
 * layer whatever its shape, and nhat = grad(psi)/|grad(psi)| is their unit normal. The layer is
 * stretched by displacing each of its points into the complex plane along that normal,
 *
 *     x~ = x + i W,     W = alpha psi^n nhat,
 *
 * whose Jacobian is J = I + i grad(W).
 *
 * alpha is the total imaginary displacement across the layer, so an outgoing wave that crosses it
 * and returns is attenuated by exp(-2 k alpha).
 *
 * W is discontinuous between elements, as it is built from the gradient of psi, so it is
 * interpolated onto a continuous space once and differentiated there. The class evaluates W as a
 * vector coefficient for that interpolation and afterwards serves grad(W).
 */
class MFEMPMLStretchVector : public mfem::VectorCoefficient
{
public:
  MFEMPMLStretchVector(mfem::ParMesh & mesh,
                       const mfem::Array<int> & pml_attributes,
                       double decay_coefficient,
                       double decay_polynomial,
                       MPI_Comm comm);

  int dim() const { return _dim; }

  using mfem::VectorCoefficient::Eval;
  void Eval(mfem::Vector & W,
            mfem::ElementTransformation & transformation,
            const mfem::IntegrationPoint & integration_point) override;

  /// grad(W), the imaginary part of the stretch Jacobian J = I + i grad(W).
  void stretchGradient(mfem::ElementTransformation & transformation,
                       mfem::DenseMatrix & gradient) const
  {
    _W_h1.GetVectorGradient(transformation, gradient);
  }

private:
  /// Solve for the harmonic coordinate, holding degrees of freedom outside the layer at zero and
  /// those on its outer surface at one.
  void solveHarmonicCoordinate(mfem::ParMesh & mesh,
                               const mfem::Array<int> & pml_attributes,
                               MPI_Comm comm);

  /// Error out where the harmonic coordinate stagnates, leaving the stretch direction undefined.
  void checkFoliation(mfem::ParMesh & mesh,
                      const mfem::Array<int> & pml_attributes,
                      MPI_Comm comm) const;

  const int _dim;
  const double _decay_coefficient;
  const double _decay_polynomial;
  mfem::H1_FECollection _h1_coll;
  mfem::ParFiniteElementSpace _h1_scalar_fes;
  mfem::ParFiniteElementSpace _h1_vector_fes;
  mfem::ParGridFunction _psi;
  mfem::ParGridFunction _W_h1;
};

#endif
