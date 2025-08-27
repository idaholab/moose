//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryFluxBase.h"

/**
 * Characteristic-based outflow for SWE using 1D Riemann invariants along the boundary normal.
 *
 * Subcritical outflow (un < c): preserves outgoing characteristic from interior (R+) and sets
 * incoming characteristic (R-) from target downstream depth/velocity. Builds a ghost state and
 * computes a Rusanov flux between interior and ghost (full physical flux including pressure).
 *
 * Supercritical outflow (un >= c): extrapolates interior state (pure outflow).
 */
class SWECharacteristicOutflowExactBoundaryFlux : public BoundaryFluxBase
{
public:
  static InputParameters validParams();

  SWECharacteristicOutflowExactBoundaryFlux(const InputParameters & parameters);
  virtual ~SWECharacteristicOutflowExactBoundaryFlux();

  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        const std::vector<Real> & uvec1,
                        const RealVectorValue & dwave,
                        std::vector<Real> & flux) const override;

  virtual void calcJacobian(unsigned int iside,
                            dof_id_type ielem,
                            const std::vector<Real> & uvec1,
                            const RealVectorValue & dwave,
                            DenseMatrix<Real> & jac1) const override;

protected:
  const Real _g;
  const Real _h_eps;
  const Real _target_h;
  const Real _target_un;
  /// Scales the pressure term in the momentum flux at the boundary (0: advective-only, 1: full)
  const Real _pressure_weight;
};
