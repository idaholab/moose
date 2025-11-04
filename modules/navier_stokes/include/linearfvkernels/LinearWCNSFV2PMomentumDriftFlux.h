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
#include "LinearFVFluxKernel.h"

class RhieChowMassFlux;
class LinearFVBoundaryCondition;

/**
 * Adds drift flux kernel coming for two-phase mixture model for the linear finite volume
 * discretization
 */
class LinearWCNSFV2PMomentumDriftFlux : public LinearFVFluxKernel
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

  /// The dimension of the simulation
  const unsigned int _dim;

  /// The Rhie-Chow user object that provides us with the face velocity
  const RhieChowMassFlux & _mass_flux_provider;

  /// Dispersed phase density
  const Moose::Functor<Real> & _rho_d;

  /// Dispersed phase fraction
  const Moose::Functor<Real> & _f_d;

  /// slip velocity in direction x
  const Moose::Functor<Real> & _u_slip;
  /// slip velocity in direction y
  const Moose::Functor<Real> * const _v_slip;
  /// slip velocity in direction z
  const Moose::Functor<Real> * const _w_slip;

  /// The index of the momentum component
  const unsigned int _index;

  /// The face interpolation method for the density
  const Moose::FV::InterpMethod _density_interp_method;

  /// Face flux
  Real _face_flux;
  /// Advected coefficients
  std::pair<Real, Real> _velocity_interp_coeffs;
};
