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
NLCurlCurlIntegrator::NLCurlCurlIntegrator(mfem::Coefficient & k,
                                           mfem::Coefficient & curlu_dk_dcurlu,
                                           mfem::VectorCoefficient & curlu_vec,
                                           const mfem::IntegrationRule * ir)
  : _curlu_hat_coef(curlu_vec, 1e-16),
    _curlu_hat_otimes_curlu_hat(_curlu_hat_coef, _curlu_hat_coef),
    _jac2_matrix_coef(curlu_dk_dcurlu, _curlu_hat_otimes_curlu_hat),
    _curlcurl_res_integ(k, ir),
    _curlcurl_jac2_integ(_jac2_matrix_coef, ir)
{
  _curlcurl_jac_integ.AddIntegrator(&_curlcurl_res_integ);
  _curlcurl_jac_integ.AddIntegrator(&_curlcurl_jac2_integ);
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
