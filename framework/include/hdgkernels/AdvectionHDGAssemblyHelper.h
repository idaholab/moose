//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HDGAssemblyHelper.h"

/**
 * Shared assembly for two-field hybridized DG discretizations of the advection equation.
 *
 * Derived helpers provide the face velocity appropriate to their flow discretization.
 */
template <typename Base>
class AdvectionHDGAssemblyHelperTempl : public Base
{
public:
  static InputParameters validParams();

  AdvectionHDGAssemblyHelperTempl(const MooseObject * moose_obj,
                                 MooseVariableDependencyInterface * mvdi,
                                 const TransientInterface * ti,
                                 SystemBase & sys,
                                 const Assembly & assembly,
                                 THREAD_ID tid,
                                 const std::set<SubdomainID> & block_ids,
                                 const std::set<BoundaryID> & boundary_ids);

  virtual void scalarVolume() override;
  virtual void scalarFace() override;
  virtual void lmFace() override;
  virtual void scalarDirichlet(const Moose::Functor<Real> & dirichlet_value) override;

  /// Prescribes an outflow condition for the facet scalar.
  void lmOutflow();

protected:
  using Base::_current_elem;
  using Base::_current_side;
  using Base::_grad_scalar_phi;
  using Base::_JxW;
  using Base::_JxW_face;
  using Base::_lm_phi_face;
  using Base::_lm_re;
  using Base::_lm_u_sol;
  using Base::_normals;
  using Base::_q_point_face;
  using Base::_qrule;
  using Base::_qrule_face;
  using Base::_scalar_phi_face;
  using Base::_scalar_re;
  using Base::_ti;
  using Base::_u_sol;

  /**
   * @returns The velocity on the current face quadrature point
   */
  virtual ADRealVectorValue faceVelocity(unsigned int qp) const = 0;

  /**
   * Computes the upwind face flux.
   * @param qp The quadrature point index
   * @param face_value The advected quantity evaluated on the face
   */
  ADReal computeFlux(unsigned int qp, const ADReal & face_value) const;

  /// Cell-interior velocity used in the volume advection term.
  const ADMaterialProperty<RealVectorValue> & _velocity;

  /// Constant coefficient multiplying the advected scalar, such as density.
  const Real _coeff;
};

using AdvectionHDGAssemblyHelper = AdvectionHDGAssemblyHelperTempl<HDGAssemblyHelper>;
