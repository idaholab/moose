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
  /// Construct the nonlinear convective heat flux residual and Jacobian action.
  NLBoundaryConvectiveHeatFluxIntegrator(mfem::Coefficient & k,
                                         mfem::Coefficient & dk_du,
                                         mfem::Coefficient & duinf_du,
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
  /// Coefficient for the shifted state, u - u_inf.
  mfem::SumCoefficient _shifted_gf_coef;
  /// Coefficient for the inwards flux contribution, k(u) * u_inf.
  mfem::ProductCoefficient _k_uinf_coef;
  /// Coefficient for the dk/du contribution, dk/du * (u - u_inf).
  mfem::ProductCoefficient _net_flux_du_coef;
  /// Coefficient for the d u_inf / du contribution, k(u) * d u_inf / du.
  mfem::ProductCoefficient _k_duinf_du_coef;
  /// Negated coefficient for the d u_inf / du Jacobian term, -k(u) * d u_inf / du.
  mfem::ProductCoefficient _duinf_du_flux_coef;
  /// Boundary load integrator for the inwards flux term.
  mfem::BoundaryLFIntegrator _inwards_flux;
  /// Boundary mass integrator for the outwards flux term.
  mfem::BoundaryMassIntegrator _outwards_flux;
  /// Jacobian contribution from the h(T) term.
  mfem::BoundaryMassIntegrator _jacobian_k_component;
  /// Jacobian contribution from the dh/dT term.
  mfem::BoundaryMassIntegrator _jacobian_dk_du_component;
  /// Jacobian contribution from the dT_inf/dT term.
  mfem::BoundaryMassIntegrator _jacobian_duinf_du_component;
  /// Sum of the boundary mass integrators forming the Jacobian action.
  mfem::SumIntegrator _jacobian_action{0};
};
}

#endif
