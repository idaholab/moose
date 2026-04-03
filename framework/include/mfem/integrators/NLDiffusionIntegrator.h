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
 * (k(u) \vec \nabla u, \vec \nabla v)
 * \f]
 */
class NLDiffusionIntegrator : public mfem::NonlinearFormIntegrator
{
public:
  NLDiffusionIntegrator(mfem::Coefficient & k,
                        mfem::Coefficient & dk_du,
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
  mfem::DiffusionIntegrator _diffusion_integ;
  mfem::ConstantCoefficient _neg_one{-1.0};
  mfem::GradientGridFunctionCoefficient _grad_trial;
  mfem::ScalarVectorProductCoefficient _neg_grad_trial;
  mfem::ScalarVectorProductCoefficient _neg_dk_du_grad_trial;
  mfem::MixedScalarWeakDivergenceIntegrator _weak_div_integ;
  mfem::SumIntegrator _sum{0};
};
}

#endif
