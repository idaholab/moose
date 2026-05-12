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
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{
/**
 * \f[
 * (k(\nabla x \vec u) \vec \nabla x \vec u, \vec \nabla x \vec v)
 * \f]
 */
class NLCurlCurlIntegrator : public mfem::NonlinearFormIntegrator
{
public:
  NLCurlCurlIntegrator(mfem::Coefficient & k,
                       mfem::VectorCoefficient & dk_dcu,
                       const mfem::GridFunction * gf,
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
  mfem::CurlGridFunctionCoefficient _curl_trial;
  mfem::InnerProductCoefficient _a2;
  mfem::SumCoefficient _jac_coeff;
  mfem::CurlCurlIntegrator _curlcurl_res_integ, _curlcurl_jac_integ;
};
}

#endif
