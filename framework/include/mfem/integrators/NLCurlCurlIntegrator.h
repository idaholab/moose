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
 * \f[
 * (k(|\vec \nabla \times \vec u|) \vec \nabla \times \vec u, \vec \nabla \times \vec v)
 * \f]
 */
class NLCurlCurlIntegrator : public mfem::NonlinearFormIntegrator
{
public:
  NLCurlCurlIntegrator(mfem::Coefficient & k,
                       mfem::Coefficient & curlu_dk_dcurlu,
                       mfem::VectorCoefficient & curlu_vec,
                       const mfem::IntegrationRule * ir = nullptr);

  virtual void AssembleElementVector(const mfem::FiniteElement & el,
                                     mfem::ElementTransformation & Tr,
                                     const mfem::Vector & elfun,
                                     mfem::Vector & elvect) override;
  virtual void AssembleElementGrad(const mfem::FiniteElement & el,
                                   mfem::ElementTransformation & Tr,
                                   const mfem::Vector & elfun,
                                   mfem::DenseMatrix & elmat) override;

protected:
  mfem::NormalizedVectorCoefficient _curlu_hat_coef;         // curlu_hat = curlu/|curl u|
  mfem::OuterProductCoefficient _curlu_hat_otimes_curlu_hat; // (curlu_hat \otimes curlu_hat)
  mfem::ScalarMatrixProductCoefficient
      _jac2_matrix_coef; //|curl u| dk/d|curl u| (curlu_hat \otimes curlu_hat)
  mfem::CurlCurlIntegrator _curlcurl_res_integ;      // (k(|curl u|) curl u, curl phi_j)
  mfem::CurlCurlIntegrator
      _curlcurl_jac2_integ; // |curl u| dk/d|curl u| (curlu_hat.curl phi_i, curlu_hat.curl phi_j)
  mfem::SumIntegrator _curlcurl_jac_integ{
      0}; // (k(|curl u|) curl phi_i, curl phi_j) + |curl u| dk/d|curl u| (curlu_hat.curl phi_i,
          // curlu_hat.curl phi_j)
};
}

#endif
