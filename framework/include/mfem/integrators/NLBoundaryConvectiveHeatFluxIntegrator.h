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

#pragma once
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{
/**
 * \f[
 *  (k(u) (u-u_\infty), v)
 * \f]
 */
class NLBoundaryConvectiveHeatFluxIntegrator : public mfem::NonlinearFormIntegrator
{
public:
  NLBoundaryConvectiveHeatFluxIntegrator(mfem::Coefficient & k,
                                         mfem::Coefficient & dk_du,
                                         mfem::Coefficient & gf_offset,
                                         mfem::Coefficient & gf);

  virtual void AssembleElementVector(const mfem::FiniteElement & el,
                                     mfem::ElementTransformation & Tr,
                                     const mfem::Vector & elfun,
                                     mfem::Vector & elvect) override;
  virtual void AssembleElementGrad(const mfem::FiniteElement & el,
                                   mfem::ElementTransformation & Tr,
                                   const mfem::Vector & elfun,
                                   mfem::DenseMatrix & elmat) override;

protected:
  mfem::SumCoefficient _shifted_gf_coef;       // (u_inf-u)
  mfem::ProductCoefficient _k_uinf_coef;       // k(u)*u_inf
  mfem::ProductCoefficient _net_flux_du_coef;  // dk/du*(u-u_inf)
  mfem::BoundaryLFIntegrator _inwards_flux;    // (k(u) u_inf, v)
  mfem::BoundaryMassIntegrator _outwards_flux; // (k(u) u, v)
  mfem::BoundaryMassIntegrator _jacobian_k_component;
  mfem::BoundaryMassIntegrator _jacobian_dk_du_component;
  mfem::SumIntegrator _jacobian_action{0};
};
}

#endif
