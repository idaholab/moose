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
 * Characteristic-inspired outflow boundary flux for SWE.
 *
 * Implements an advective-only flux using the interior state projected on the
 * outward normal. This avoids injecting hydrostatic pressure at the boundary
 * and reduces spurious reflections.
 */
class SWECharacteristicOutflowBoundaryFlux : public BoundaryFluxBase
{
public:
  static InputParameters validParams();

  SWECharacteristicOutflowBoundaryFlux(const InputParameters & parameters);
  virtual ~SWECharacteristicOutflowBoundaryFlux();

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
  const Real _h_eps;
  /// If true, emit flux only when un>0 (outflow); otherwise zero flux for inflow
  const bool _outflow_only;
  /// Optional ramp time for turning on the advective flux smoothly starting from t=0
  const Real _ramp_time;
  /// Optional number of initial time steps to suppress outflow flux entirely
  const unsigned int _ramp_steps;
};
