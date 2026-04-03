//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "NLBoundaryConvectiveHeatFluxIntegrator.h"

namespace Moose::MFEM
{
NLBoundaryConvectiveHeatFluxIntegrator::NLBoundaryConvectiveHeatFluxIntegrator(
    mfem::Coefficient & k,
    mfem::Coefficient & dk_du,
    mfem::Coefficient & gf_offset,
    mfem::Coefficient & gf)
  : _shifted_gf_coef(gf, gf_offset, 1.0, -1.0), // u-u_inf
    _k_uinf_coef(k, gf_offset),                 // k(u)*u_inf
    _net_flux_du_coef(dk_du, _shifted_gf_coef), // dk/du*(u-u_inf)
    _inwards_flux(_k_uinf_coef),
    _outwards_flux(k),
    _jacobian_k_component(k),
    _jacobian_dk_du_component(_net_flux_du_coef)
{
  _jacobian_action.AddIntegrator(&_jacobian_k_component);
  _jacobian_action.AddIntegrator(&_jacobian_dk_du_component);
}

void
NLBoundaryConvectiveHeatFluxIntegrator::AssembleElementVector(const mfem::FiniteElement & el,
                                                              mfem::ElementTransformation & Tr,
                                                              const mfem::Vector & elfun,
                                                              mfem::Vector & elvect)
{
  mfem::Vector ext_flux_elvect; // vector to store flux coming from outside of domain
  _outwards_flux.AssembleElementVector(el, Tr, elfun, elvect);   // (k(u)*u, v)
  _inwards_flux.AssembleRHSElementVect(el, Tr, ext_flux_elvect); // (k(u)*u_inf, v)
  elvect -= ext_flux_elvect;                                     // (k(u)*(u-u_inf),v)
}

void
NLBoundaryConvectiveHeatFluxIntegrator::AssembleElementGrad(const mfem::FiniteElement & el,
                                                            mfem::ElementTransformation & Tr,
                                                            const mfem::Vector & elfun,
                                                            mfem::DenseMatrix & elmat)
{
  // (k + dk_du (u-u_inf)) phi, v);
  _jacobian_action.AssembleElementGrad(el, Tr, elfun, elmat); //
}
}

#endif
