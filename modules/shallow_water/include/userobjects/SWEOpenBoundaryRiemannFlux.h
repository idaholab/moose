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
#include "InternalSideFluxBase.h"

/**
 * Open/outflow boundary flux for SWE using a ghost-state Riemann approach.
 *
 * - Builds a ghost state by setting the incoming characteristic from a farfield
 *   condition (stage or dry) and preserving outgoing interior characteristic.
 * - Calls an embedded HLLC (or HLL) Riemann solver along the boundary normal
 *   to compute a full physical flux (with pressure), mirroring interior faces.
 */
class SWEOpenBoundaryRiemannFlux : public BoundaryFluxBase
{
public:
  static InputParameters validParams();

  SWEOpenBoundaryRiemannFlux(const InputParameters & parameters);
  virtual ~SWEOpenBoundaryRiemannFlux();

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
  enum class FarfieldMode
  {
    Stage,
    Dry
  };

  const Real _g;
  const Real _h_eps;
  const FarfieldMode _mode;
  const Real _eta_infty;
  const Real _un_infty;
  const Real _ut_infty;
  const bool _use_hllc;
  const bool _allow_backflow;

  // Reuse the exact same internal-face numerical flux (HLLC/HLL) as the interior
  const InternalSideFluxBase & _riemann_flux;
};
