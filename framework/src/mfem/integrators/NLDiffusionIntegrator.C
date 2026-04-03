//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "NLDiffusionIntegrator.h"

namespace Moose::MFEM
{
NLDiffusionIntegrator::NLDiffusionIntegrator(mfem::Coefficient & k,
                                             mfem::Coefficient & dk_du,
                                             const mfem::GridFunction * gf,
                                             const mfem::IntegrationRule * ir)
  : _diffusion_integ(k, ir),
    _grad_trial(gf),
    _neg_grad_trial(_neg_one, _grad_trial),
    _neg_dk_du_grad_trial(dk_du, _neg_grad_trial),
    _weak_div_integ(_neg_grad_trial)
{
  _sum.AddIntegrator(&_diffusion_integ);
  _sum.AddIntegrator(&_weak_div_integ);
}

void
NLDiffusionIntegrator::AssembleElementVector(const mfem::FiniteElement & el,
                                             mfem::ElementTransformation & Tr,
                                             const mfem::Vector & elfun,
                                             mfem::Vector & elvect)
{
  _diffusion_integ.AssembleElementVector(el, Tr, elfun, elvect);
}

void
NLDiffusionIntegrator::AssembleElementGrad(const mfem::FiniteElement & el,
                                           mfem::ElementTransformation & Tr,
                                           const mfem::Vector & elfun,
                                           mfem::DenseMatrix & elmat)
{
  _sum.AssembleElementGrad(el, Tr, elfun, elmat);
}
}

#endif
