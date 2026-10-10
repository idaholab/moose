//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MathFVUtils.h"
#include "LinearWCNSFV2PDriftFluxBase.h"

class LinearFVBoundaryCondition;

/**
 * Diffusion (drift) stress of the two-phase mixture model for the linear finite volume
 * discretization.
 *
 * Summing the phase momentum equations and substituting \f$ u_k = u_m + u_{Mk} \f$, where
 * \f$ u_{Mk} = u_k - u_m \f$ is the diffusion velocity of phase \f$ k \f$, leaves one term
 * beyond the single phase equation,
 *
 * \f[
 *   \nabla \cdot \tau_{Dm}, \qquad \tau_{Dm} = -\sum_k \alpha_k \rho_k u_{Mk} u_{Mk}
 * \f]
 *
 * on the right hand side, i.e. \f$ +\nabla \cdot \left( \sum_k \alpha_k \rho_k u_{Mk}
 * u_{Mk} \right) \f$ on the left hand side, which is the form this kernel assembles. For a
 * single dispersed phase the sum collapses onto the slip velocity \f$ u_s = u_d - u_c \f$,
 *
 * \f[
 *   \sum_k \alpha_k \rho_k u_{Mk} u_{Mk}
 *     = \frac{\beta_d \beta_c}{\rho_m} \, u_s \otimes u_s,
 *   \qquad \beta_d = \alpha \rho_d, \quad \beta_c = (1 - \alpha) \rho_c
 * \f]
 *
 * The coefficient is exact, not the dilute limit \f$ \beta_d \f$ obtained by letting
 * \f$ \beta_c / \rho_m \to 1 \f$. See Manninen, Taivassalo and Kallio, VTT Publications 288
 * (1996), equations (18), (21) and (76).
 */
class LinearWCNSFV2PMomentumDriftFlux : public LinearWCNSFV2PDriftFluxBase
{
public:
  static InputParameters validParams();
  LinearWCNSFV2PMomentumDriftFlux(const InputParameters & params);

  virtual Real computeElemMatrixContribution() override;

  virtual Real computeNeighborMatrixContribution() override;

  virtual Real computeElemRightHandSideContribution() override;

  virtual Real computeNeighborRightHandSideContribution() override;

  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition &) override
  {
    return 0;
  }
  virtual Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) override;

  /**
   * Set the current FaceInfo object. We override this here to make sure the face velocity
   * evaluation happens only once and that it can be reused for the matrix and right hand side
   * contributions.
   * @param face_info The face info which will be used as current face info
   */
  virtual void setupFaceData(const FaceInfo * face_info) override;

protected:
  /// Computes the matrix contribution of the advective flux on the element side of current face
  /// when the face is an internal face (doesn't have associated boundary conditions).
  Real computeInternalAdvectionElemMatrixContribution();

  /// Computes the matrix contribution of the advective flux on the neighbor side of current face
  /// when the face is an internal face (doesn't have associated boundary conditions).
  Real computeInternalAdvectionNeighborMatrixContribution();

  /// Compute the face flux
  void computeFlux();

  /// The deferred correction added to the right hand side of the element row. It is the difference
  /// between the implicit surrogate the matrix carries and the exact flux, so that the two cancel
  /// once the fixed point iteration converges.
  Real deferredCorrection() const;

  /// Continuous phase density
  const Moose::Functor<Real> & _rho_c;

  /// The index of the momentum component
  const unsigned int _index;

  /// The face interpolation method for the density
  const Moose::FV::InterpMethod _density_interp_method;

  /// Face flux
  Real _face_flux;

  /// The face flux divided by the slip velocity component this kernel acts on, i.e. the diffusion
  /// stress coefficient times the normal slip velocity. Used as the scale of the implicit surrogate
  Real _slip_mass_flux;

  /// The exact coefficient of the diffusion stress, beta_d beta_c / rho_m, at the given argument
  template <typename SpaceArg>
  Real diffusionStressCoefficient(const SpaceArg & arg, const Moose::StateArg & state) const
  {
    return NS::diffusionStressCoefficient(_f_d(arg, state), _rho_d(arg, state), _rho_c(arg, state));
  }

  /// Coefficient of the implicit surrogate used for the deferred correction. Chosen for
  /// convergence only, see setupFaceData; the converged solution does not depend on it.
  Real _gamma;
};
