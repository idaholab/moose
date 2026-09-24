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
#include "MFEMVectorMagnitudeCoefficient.h"
#include "MooseError.h"

namespace Moose::MFEM
{
/**
 * Matrix coefficient for the Jacobian of NLCurlCurlIntegrator.
 *
 * Produces the matrix
 *   k(|curl u|) I + |curl u| dk/d|curl u| (curl u_hat \otimes curl u_hat)
 */
class NLCurlCurlJacMatrixCoefficient : public mfem::MatrixCoefficient
{
public:
  NLCurlCurlJacMatrixCoefficient(mfem::Coefficient & k,
                                 mfem::Coefficient & curlu_dk_dcurlu,
                                 mfem::VectorCoefficient & curlu_vec,
                                 mfem::real_t curlu_zero_tol);

  void Eval(mfem::DenseMatrix & K,
            mfem::ElementTransformation & T,
            const mfem::IntegrationPoint & ip) override;
  void SetTime(mfem::real_t t) override;

protected:
  mfem::Coefficient & _k_coef;
  mfem::Coefficient & _curlu_dk_dcurlu_coef;
  const mfem::real_t _curlu_zero_tol;
  mfem::NormalizedVectorCoefficient _curlu_hat_coef;
};

/**
 * \f[
 * (k(|\vec \nabla \times \vec u|) \vec \nabla \times \vec u, \vec \nabla \times \vec v)
 * \f]
 */
class NLCurlCurlIntegrator : public mfem::NonlinearFormIntegrator
{
public:
  using mfem::NonlinearFormIntegrator::AssemblePA;

  NLCurlCurlIntegrator(mfem::Coefficient & k,
                       mfem::Coefficient & curlu_dk_dcurlu,
                       mfem::Coefficient & dk_dcurlu,
                       mfem::VectorCoefficient & curlu_vec,
                       mfem::real_t curlu_zero_tol = 1e-32,
                       const mfem::IntegrationRule * ir = nullptr);

  virtual void AssembleElementVector(const mfem::FiniteElement & el,
                                     mfem::ElementTransformation & Tr,
                                     const mfem::Vector & elfun,
                                     mfem::Vector & elvect) override;
  virtual void AssembleElementGrad(const mfem::FiniteElement & el,
                                   mfem::ElementTransformation & Tr,
                                   const mfem::Vector & elfun,
                                   mfem::DenseMatrix & elmat) override;

  void AssemblePA(const mfem::FiniteElementSpace & fes) override;
  void AssembleGradPA(const mfem::Vector & x, const mfem::FiniteElementSpace & fes) override;
  void AddMultPA(const mfem::Vector & x, mfem::Vector & y) const override;
  void AddMultGradPA(const mfem::Vector & x, mfem::Vector & y) const override;
  void AssembleGradDiagonalPA(mfem::Vector & diag) const override;

  /// Performs the setup for both AssemblePA and AssembleGradPA, then returns
  /// the integration rule we need for projecting coefficients.
  const mfem::IntegrationRule * PreAssemblySetup(const mfem::FiniteElementSpace & fes);

protected:
  mfem::CurlCurlIntegrator _curlcurl_res_integ; // (k(|curl u|) curl u, curl phi_j)
  NLCurlCurlJacMatrixCoefficient _curlcurl_jac_matrix_coef;
  mfem::CurlCurlIntegrator _curlcurl_jac_integ;

  // Extra stuff we need for the PA extension
  mfem::Vector _pa_res_data, _pa_grad_data;
  const mfem::DofToQuad * _mapsO;       ///< Not owned. DOF-to-quad map, open.
  const mfem::DofToQuad * _mapsC;       ///< Not owned. DOF-to-quad map, closed.
  const mfem::GeometricFactors * _geom; ///< Not owned
  int _dim, _ne, _nq, _dofs1D, _quad1D;

  /// Since k() is a scalar function, the matrices we form will always be symmetric,
  /// since the two matrices we form are k(s) J^TJ and k'(s)/s * J^T * (curl u) * (curl u)^T J,
  /// both of which are symmetric.
  const bool _symmetric = true; ///< False if using a nonsymmetric matrix coefficient
  /// Number of matrix elements to store per quadpoint. Since we only support 3D matrices
  /// with scalar k, we can set this to 6.
  const int _ndata = 6;

  /// Coefficient for k(s)
  mfem::Coefficient & _k_coef;
  /// Coefficient for k'(s) / s, with s = |curl u|
  mfem::Coefficient & _dk_dcurlu_coef;
  /// Coefficient for the curl of u
  mfem::VectorCoefficient & _curlu_vec;

  /// Quadrature space the solution-dependent coefficients are projected onto. Owned, and
  /// rebuilt whenever the mesh or integration rule it was built from changes.
  std::unique_ptr<mfem::QuadratureSpace> _qspace;
  /// What _qspace was built from, used to detect that it has gone stale. The sequence counter
  /// catches in-place refinement, which leaves the mesh pointer unchanged.
  const mfem::Mesh * _qspace_mesh = nullptr;
  long _qspace_mesh_sequence = -1;
  const mfem::IntegrationRule * _qspace_ir = nullptr;
};
}

#endif
