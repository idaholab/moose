//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
//*//
#ifdef MOOSE_MFEM_ENABLED

#include "NLCurlCurlIntegrator.h"

namespace Moose::MFEM
{
NLCurlCurlIntegrator::NLCurlCurlIntegrator(mfem::Coefficient & k,
                                           mfem::Coefficient & dk_dcurlu,
                                           const mfem::GridFunction * gf,
                                           const mfem::IntegrationRule * ir)
  : _curlu_vec(gf),
    _curlu(_curlu_vec),
    _curlu_dk_dcurlu_coef(_curlu, dk_dcurlu),
    _k_plus_curlu_dk_dcurlu_coef(k, _curlu_dk_dcurlu_coef),
    _curlcurl_res_integ(k, ir),
    _curlcurl_jac_integ(_k_plus_curlu_dk_dcurlu_coef, ir)
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
};

#endif
