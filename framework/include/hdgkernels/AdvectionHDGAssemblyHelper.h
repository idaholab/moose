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
 * Shared assembly for two-field hybridized DG discretizations of the advection equation.
 *
 * Derived helpers provide the face velocity appropriate to their flow discretization.
 */
class AdvectionHDGAssemblyHelper : public IPHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  AdvectionHDGAssemblyHelper(const MooseObject * moose_obj,
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

  /// Velocity in the element interior.
  const ADMaterialProperty<RealVectorValue> & _velocity;

  /// Constant multiplying the advected scalar.
  const Real _coeff;
};
