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

/**
 * Diagonal perfectly-matched-layer matrix coefficient. Evaluates the complex Cartesian coordinate
 * stretch dxs(x) at an integration point and returns the real or imaginary part of one of the two
 * PML tensors:
 *   - C1 = detJ^{-1} J^T J   (used by the curl-curl term; scalar in 2D, diagonal in 3D)
 *   - C2 = detJ (J^T J)^{-1} (used by the vector FE mass term; diagonal, size dim)
 * The geometry (inner boundary comp_domain_bdr and per-side PML length) is supplied by the kernel,
 * derived from the mesh.
 */
class MFEMPMLDiagMatrixCoefficient : public mfem::VectorCoefficient
{
public:
  enum Tensor
  {
    C1,
    C2
  };
  enum Part
  {
    RE,
    IM
  };

  MFEMPMLDiagMatrixCoefficient(int dim,
                               const mfem::Array2D<double> & comp_domain_bdr,
                               const mfem::Array2D<double> & length,
                               double decay_coefficient,
                               double decay_polynomial,
                               Tensor tensor,
                               Part part)
    : mfem::VectorCoefficient(tensor == C1 ? (dim == 2 ? 1 : dim) : dim),
      _dim(dim),
      _comp_domain_bdr(comp_domain_bdr),
      _length(length),
      _decay_coefficient(decay_coefficient),
      _decay_polynomial(decay_polynomial),
      _tensor(tensor),
      _part(part)
  {
  }

  using mfem::VectorCoefficient::Eval;

  void Eval(mfem::Vector & K,
            mfem::ElementTransformation & T,
            const mfem::IntegrationPoint & ip) override;

private:
  /// Fill dxs[i] with the complex stretch factor in each Cartesian direction at point x.
  void stretchFunction(const mfem::Vector & x, std::vector<std::complex<double>> & dxs) const;

  const int _dim;
  const mfem::Array2D<double> _comp_domain_bdr;
  const mfem::Array2D<double> _length;
  const double _decay_coefficient;
  const double _decay_polynomial;
  const Tensor _tensor;
  const Part _part;
};

/**
 * Base class for PML-stretched complex bilinear-form kernels. Derives
 * the Cartesian PML geometry from the mesh (outer bounding box and the interior/non-PML region's
 * bounding box) and applies the coordinate stretch as an on-the-fly complex coefficient scaling a
 * base scalar coefficient. Subclasses supply the operator integrator and the stretch tensor
 * (C1 for curl-curl, C2 for vector FE mass). The PML region is this kernel's block.
 */
class MFEMPMLKernel : public MFEMComplexKernel
{
public:
  static InputParameters validParams();
  MFEMPMLKernel(const InputParameters & parameters, MFEMPMLDiagMatrixCoefficient::Tensor tensor);

  mfem::BilinearFormIntegrator * getRealBFIntegrator() override;
  mfem::BilinearFormIntegrator * getImagBFIntegrator() override;

protected:
  /// Build the operator integrator from the (base-coefficient-scaled) diagonal coefficient.
  virtual mfem::BilinearFormIntegrator * makeIntegrator(mfem::VectorCoefficient & coef) = 0;

  /// Bounding box of the interior (non-PML) region = inner PML boundary, reduced across ranks.
  mfem::Array2D<double> getCompDomainBoundary();
  /// Per-axis, per-side PML thickness from the mesh bounding box and the inner PML boundary.
  mfem::Array2D<double> getLength(const mfem::Array2D<double> & comp_domain_bdr);

  /// Base scalar coefficient (e.g. reluctivity 1/mu or mass -omega^2 eps).
  mfem::Coefficient & _base_coef;
  const double _decay_coefficient;
  const double _decay_polynomial;

  const int _dim;
  /// Inner PML boundary (per axis, low/high side), i.e. where the PML starts.
  const mfem::Array2D<double> _comp_domain_bdr;
  /// PML thickness per axis and side (0 on a side with no PML).
  const mfem::Array2D<double> _length;

  /// Real and imaginary parts of the diagonal stretch tensor, scaled by the base coefficient.
  MFEMPMLDiagMatrixCoefficient _pml_re;
  MFEMPMLDiagMatrixCoefficient _pml_im;
  mfem::ScalarVectorProductCoefficient _scaled_re;
  mfem::ScalarVectorProductCoefficient _scaled_im;
};

#endif