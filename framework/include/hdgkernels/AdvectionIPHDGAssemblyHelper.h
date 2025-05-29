//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IPHDGAssemblyHelper.h"

/**
 * Implements all the methods for assembling a hybridized interior penalty discontinuous Galerkin
 * (IPDG-H), which is a type of HDG method, discretization of the advection equation. These routines
 * may be called by both HDG kernels and integrated boundary conditions.
 */
class AdvectionIPHDGAssemblyHelper : public IPHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  AdvectionIPHDGAssemblyHelper(const MooseObject * const moose_obj,
                               MooseVariableDependencyInterface * const mvdi,
                               const TransientInterface * const ti,
                               SystemBase & sys,
                               const Assembly & assembly,
                               const THREAD_ID tid,
                               const std::set<SubdomainID> & block_ids,
                               const std::set<BoundaryID> & boundary_ids);

  /**
   * Computes a local residual vector for the weak form:
   * (Dq, grad(w)) - (f, w)
   * where D is the diffusivity, w are the test functions associated with the scalar field, and f is
   * a forcing function
   */
  virtual void scalarVolume() override;

  /**
   * Computes a local residual vector for the weak form:
   * -<Dq*n, w> + <\tau * (u - \hat{u}) * n * n, w>
   */
  virtual void scalarFace() override;

  /**
   * Computes a local residual vector for the weak form:
   * -<Dq*n, \mu> + <\tau * (u - \hat{u}) * n * n, \mu>
   */
  virtual void lmFace() override;

  /**
   * Weakly imposes a Dirichlet condition for the scalar field in the scalar field equation
   */
  virtual void scalarDirichlet(const Moose::Functor<Real> & dirichlet_value) override;

  /**
   * prescribes an outflow condition
   */
  void lmOutflow();

  /// The velocity in the element interior
  const ADMaterialProperty<RealVectorValue> & _velocity;

  /// The velocity on the element faces
  const ADMaterialProperty<RealVectorValue> & _face_velocity;

  /// The quantity we are advecting, e.g. something like a density. If \p _self_advection is true
  /// then the advected quantity value is this \p _coeff value multipled by the
  /// variable/side_variable pair (for element upwind/downwind of the face respectively)
  const Real _coeff;

  /// Whether this kernel should advect itself, e.g. its variable/side_variable pair (for element
  /// upwind/downwind of the face respectively). If false, we will advect just the \p _coeff value
  const bool _self_advection;
};
