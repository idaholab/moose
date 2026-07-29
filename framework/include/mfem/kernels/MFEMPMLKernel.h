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

#include "MFEMComplexKernel.h"
#include "MFEMPMLStretch.h"

/**
 * Perfectly matched layer tensor coefficient.
 *
 * The radial stretch is diagonal in the local radial and tangential frame, so the tensor in
 * Cartesian coordinates is
 *
 *     M = lambda_t I + (lambda_r - lambda_t) rhat (x) rhat,
 *
 * whose eigenvalues follow from the radial and tangential stretch factors J_r and J_t and their
 * determinant det(J) = J_r J_t^(d-1).
 *
 * Which tensor applies is set by the quantity the bilinear form integrates, not by the operator
 * using it: pulling the weak form back from stretched to physical coordinates gives one factor for
 * an integrand holding the curl of the field and another for one holding the field itself.
 *
 *     CURL  = det(J)^-1 J^T J    lambda_r = J_r^2/det, lambda_t = J_t^2/det
 *     FIELD = det(J) (J^T J)^-1  lambda_r = det/J_r^2, lambda_t = det/J_t^2
 *
 * The result is scaled by a base scalar coefficient, and only its real or imaginary part is
 * returned, as the complex system is assembled from two real bilinear forms.
 */
class MFEMPMLMatrixCoefficient : public mfem::MatrixCoefficient
{
public:
  enum Tensor
  {
    CURL,
    FIELD
  };
  enum Part
  {
    RE,
    IM
  };

  MFEMPMLMatrixCoefficient(const MFEMPMLStretch & stretch,
                           mfem::Coefficient & base_coefficient,
                           Tensor tensor,
                           Part part)
    : mfem::MatrixCoefficient(stretch.dim()),
      _stretch(stretch),
      _base_coefficient(base_coefficient),
      _tensor(tensor),
      _part(part)
  {
  }

  void Eval(mfem::DenseMatrix & K,
            mfem::ElementTransformation & T,
            const mfem::IntegrationPoint & ip) override;

private:
  const MFEMPMLStretch & _stretch;
  mfem::Coefficient & _base_coefficient;
  const Tensor _tensor;
  const Part _part;
};

/**
 * Scalar perfectly matched layer coefficient a/det(J). In two dimensions the curl of a vector field
 * is a scalar, so the curl curl term picks up only the inverse determinant rather than a tensor.
 */
class MFEMPMLScalarCoefficient : public mfem::Coefficient
{
public:
  MFEMPMLScalarCoefficient(const MFEMPMLStretch & stretch,
                           mfem::Coefficient & base_coefficient,
                           MFEMPMLMatrixCoefficient::Part part)
    : _stretch(stretch), _base_coefficient(base_coefficient), _part(part)
  {
  }

  double Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip) override;

private:
  const MFEMPMLStretch & _stretch;
  mfem::Coefficient & _base_coefficient;
  const MFEMPMLMatrixCoefficient::Part _part;
};

/**
 * Base class for radial perfectly matched layer complex bilinear form kernels. The layer is this
 * kernel's block, and the stretch geometry is derived from the mesh and a reference point.
 * Subclasses build the operator integrator from the appropriate coefficient.
 */
class MFEMPMLKernel : public MFEMComplexKernel
{
public:
  static InputParameters validParams();
  MFEMPMLKernel(const InputParameters & parameters, MFEMPMLMatrixCoefficient::Tensor tensor);

  mfem::BilinearFormIntegrator * getRealBFIntegrator() override;
  mfem::BilinearFormIntegrator * getImagBFIntegrator() override;

protected:
  /// Build the operator integrator for the real or imaginary part of the stretched coefficient.
  virtual mfem::BilinearFormIntegrator * makeIntegrator(MFEMPMLMatrixCoefficient::Part part) = 0;

  /// Construct the stretch from the mesh, this kernel's block and the input parameters.
  std::unique_ptr<MFEMPMLStretch> makeStretch();

  /// Base scalar coefficient, such as the reluctivity or the mass coefficient.
  mfem::Coefficient & _base_coefficient;
  /// Declared before the coefficients below, which hold a reference to it.
  std::unique_ptr<MFEMPMLStretch> _stretch;
  MFEMPMLMatrixCoefficient _matrix_re;
  MFEMPMLMatrixCoefficient _matrix_im;
  MFEMPMLScalarCoefficient _scalar_re;
  MFEMPMLScalarCoefficient _scalar_im;
};

#endif
