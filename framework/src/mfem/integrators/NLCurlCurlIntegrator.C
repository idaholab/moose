//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
//*

#ifdef MOOSE_MFEM_ENABLED

#include "NLCurlCurlIntegrator.h"

namespace Moose::MFEM
{
NLCurlCurlJacMatrixCoefficient::NLCurlCurlJacMatrixCoefficient(mfem::Coefficient & k,
                                                               mfem::Coefficient & curlu_dk_dcurlu,
                                                               mfem::VectorCoefficient & curlu_vec)
  : mfem::MatrixCoefficient(curlu_vec.GetVDim()),
    _k_coef(k),
    _curlu_dk_dcurlu_coef(curlu_dk_dcurlu),
    _curlu_hat_coef(curlu_vec, 1e-32)
{
}

void
NLCurlCurlJacMatrixCoefficient::SetTime(mfem::real_t t)
{
  MatrixCoefficient::SetTime(t);
  _k_coef.SetTime(t);
  _curlu_dk_dcurlu_coef.SetTime(t);
  _curlu_hat_coef.SetTime(t);
}

void
NLCurlCurlJacMatrixCoefficient::Eval(mfem::DenseMatrix & K,
                                     mfem::ElementTransformation & T,
                                     const mfem::IntegrationPoint & ip)
{
  const int dim = GetHeight();
  mfem::Vector curlu_hat(dim);

  _curlu_hat_coef.Eval(curlu_hat, T, ip);
  const real_t k =_k_coef.Eval(T, ip);
  const real_t curlu_dk_dcurlu = _curlu_dk_dcurlu_coef.Eval(T, ip);

  K.Diag(k, dim);
  for (const auto i : make_range(dim))
    for (const auto j : make_range(dim))
      K(i, j) += curlu_dk_dcurlu * curlu_hat(i) * curlu_hat(j);
}

NLCurlCurlIntegrator::NLCurlCurlIntegrator(mfem::Coefficient & k,
                                           mfem::Coefficient & curlu_dk_dcurlu,
                                           mfem::VectorCoefficient & curlu_vec,
                                           const mfem::IntegrationRule * ir)
  : _curlcurl_res_integ(k, ir),
    _curlcurl_jac_matrix_coef(k, curlu_dk_dcurlu, curlu_vec),
    _curlcurl_jac_integ(_curlcurl_jac_matrix_coef, ir)
{
}

void
NLCurlCurlIntegrator::AssembleElementVector(const mfem::FiniteElement & el,
                                            mfem::ElementTransformation & Tr,
                                            const mfem::Vector & elfun,
                                            mfem::Vector & elvect)
{
  _curlcurl_res_integ.AssembleElementVector(el, Tr, elfun, elvect);
}

void
NLCurlCurlIntegrator::AssembleElementGrad(const mfem::FiniteElement & el,
                                          mfem::ElementTransformation & Tr,
                                          const mfem::Vector & elfun,
                                          mfem::DenseMatrix & elmat)
{
  _curlcurl_jac_integ.AssembleElementGrad(el, Tr, elfun, elmat);
}
}

#endif
