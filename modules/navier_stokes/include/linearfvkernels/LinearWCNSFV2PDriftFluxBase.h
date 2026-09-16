//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVFluxKernel.h"
#include "NavierStokesMethods.h"

#include <set>

/**
 * Base class for the linear finite volume flux kernels that carry a flux with the slip velocity
 * of a two-phase mixture: the slip velocity components, the dispersed phase fraction and density
 * the flux is weighted with, and the boundaries the dispersed phase may cross.
 */
class LinearWCNSFV2PDriftFluxBase : public LinearFVFluxKernel
{
public:
  static InputParameters validParams();
  LinearWCNSFV2PDriftFluxBase(const InputParameters & params);

  virtual void setupFaceData(const FaceInfo * face_info) override;

protected:
  /// The slip velocity vector at the given argument
  template <typename SpaceArg>
  RealVectorValue slipVelocity(const SpaceArg & arg, const Moose::StateArg & state) const
  {
    return NS::slipVelocityVector(_u_slip, _v_slip, _w_slip, arg, state);
  }

  /// The dimension of the simulation
  const unsigned int _dim;

  /// Volume fraction of the dispersed phase
  const Moose::Functor<Real> & _f_d;
  /// Dispersed phase density
  const Moose::Functor<Real> & _rho_d;

  /// Slip velocity components; the ones a lower dimensional mesh does not carry are null
  const Moose::Functor<Real> & _u_slip;
  const Moose::Functor<Real> * const _v_slip;
  const Moose::Functor<Real> * const _w_slip;

  /// Boundaries the dispersed phase may cross
  std::set<BoundaryID> _slip_boundaries;

  /// Multiplier that keeps the normal pointing outward on boundary faces, including boundaries
  /// which are internal to the mesh
  Real _boundary_normal_factor;
};
