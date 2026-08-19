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

  virtual void AssemblePA(const mfem::FiniteElementSpace & fes) override;
  virtual void AssembleGradPA(const mfem::Vector & x,
                              const mfem::FiniteElementSpace & fes) override;
  virtual void AddMultPA(const mfem::Vector & x, mfem::Vector & y) const override;
  virtual void AddMultGradPA(const mfem::Vector & x, mfem::Vector & y) const override;
  virtual void AssembleGradDiagonalPA(mfem::Vector & diag) const override;

  // pass in pointer to a quadrature space, so that the assembly method
  // can fetch what it needs
  void PreAssemblySetup(const mfem::FiniteElementSpace & fes, mfem::QuadratureSpace *& qs);

protected:
  mfem::CurlCurlIntegrator _curlcurl_res_integ; // (k(|curl u|) curl u, curl phi_j)
  NLCurlCurlJacMatrixCoefficient _curlcurl_jac_matrix_coef;
  mfem::CurlCurlIntegrator _curlcurl_jac_integ;

  // Extra stuff we need for the PA extension
  mfem::Vector pa_res_data, pa_grad_data;
  const mfem::DofToQuad * mapsO;       ///< Not owned. DOF-to-quad map, open.
  const mfem::DofToQuad * mapsC;       ///< Not owned. DOF-to-quad map, closed.
  const mfem::GeometricFactors * geom; ///< Not owned
  const mfem::IntegrationRule * ir;
  int dim, ne, nq, dofs1D, quad1D;
  int ndata;             // number of matrix elements to store per qpoint
  bool symmetric = true; ///< False if using a nonsymmetric matrix coefficient

  // we also need to capture the coefficient for the k function and its derivative
  mfem::Coefficient & _k_coef;
  mfem::Coefficient & _dk_du_u_coef; // this is k'(s) / s
  mfem::VectorCoefficient & _curlu_vec;
};
}

#endif
