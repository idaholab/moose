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

  /// The argument the current face is evaluated at: single sided on a boundary face, central
  /// difference on an internal one
  Moose::FaceArg currentFaceArg() const;

  /**
   * The value on the current face of a coefficient of the slip flux. A boundary face takes the
   * single sided value; an internal face interpolates the values of its two cells with `method`,
   * falling back to the arithmetic average where the two do not share a sign or one vanishes, the
   * harmonic mean not being defined there.
   * @param coefficient callable evaluating the coefficient at a space argument and a state
   */
  template <typename Coefficient>
  Real faceCoefficient(const Coefficient & coefficient,
                       const Moose::StateArg & state,
                       Moose::FV::InterpMethod method) const
  {
    if (Moose::FV::onBoundary(*this, *_current_face_info))
      return coefficient(singleSidedFaceArg(_current_face_info), state);

    const Real elem_value = coefficient(makeElemArg(_current_face_info->elemPtr()), state);
    const Real neighbor_value = coefficient(makeElemArg(_current_face_info->neighborPtr()), state);
    if (elem_value * neighbor_value <= 0.0)
      method = Moose::FV::InterpMethod::Average;

    Real face_value;
    Moose::FV::interpolate(
        method, face_value, elem_value, neighbor_value, *_current_face_info, true);
    return face_value;
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
