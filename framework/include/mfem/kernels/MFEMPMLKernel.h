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

  MFEMPMLMatrixCoefficient(const MFEMPMLStretchVector & stretch,
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
  const MFEMPMLStretchVector & _stretch;
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
  MFEMPMLScalarCoefficient(const MFEMPMLStretchVector & stretch,
                           mfem::Coefficient & base_coefficient,
                           MFEMPMLMatrixCoefficient::Part part)
    : _stretch(stretch), _base_coefficient(base_coefficient), _part(part)
  {
  }

  double Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip) override;

private:
  const MFEMPMLStretchVector & _stretch;
  mfem::Coefficient & _base_coefficient;
  const MFEMPMLMatrixCoefficient::Part _part;
};

/**
 * Base class for perfectly matched layer complex bilinear form kernels. The layer is this kernel's
 * block, and the stretch geometry is derived from it alone. Subclasses build the operator
 * integrator from the appropriate coefficient.
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

  /// The stretch for this layer, declared as a vector coefficient under a reserved name the first
  /// time it is asked for and shared by every kernel acting on the same layer thereafter, so that
  /// the harmonic coordinate is solved for once rather than once per kernel.
  MFEMPMLStretchVector & getStretch();

  /// Reserved coefficient name identifying the stretch for this layer and these profile parameters.
  std::string stretchName();

  /// Base scalar coefficient, such as the reluctivity or the mass coefficient.
  mfem::Coefficient & _base_coefficient;
  /// Declared before the coefficients below, which hold a reference to it.
  MFEMPMLStretchVector & _stretch;
  MFEMPMLMatrixCoefficient _matrix_re;
  MFEMPMLMatrixCoefficient _matrix_im;
  MFEMPMLScalarCoefficient _scalar_re;
  MFEMPMLScalarCoefficient _scalar_im;
};

#endif
