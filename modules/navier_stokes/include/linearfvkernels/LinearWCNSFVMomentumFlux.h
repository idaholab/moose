//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVFluxKernel.h"
#include "INSFVRhieChowInterpolatorSegregated.h"

/**
 * Kernel that adds contributions from an advection term discretized using the finite volume method
 * to a linear system.
 */
class LinearWCNSFVMomentumFlux : public LinearFVFluxKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearWCNSFVMomentumFlux(const InputParameters & params);

  virtual void initialSetup() override;

  virtual Real computeElemMatrixContribution() override;

  virtual Real computeNeighborMatrixContribution() override;

  virtual Real computeElemRightHandSideContribution() override;

  virtual Real computeNeighborRightHandSideContribution() override;

  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) override;

  virtual Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) override;

  /**
   * Set the current FaceInfo object. We override this here to make sure the face velocity
   * evaluation happens only once and that it can be reused for the matrix and right hand side
   * contributions.
   * @param face_info The face info which will be used as current face info
   */
  virtual void setCurrentFaceInfo(const FaceInfo * face_info) override;

protected:
  /// Computes the matrix contribution on of the advective flux on the element side of current face
  /// when the face is an internal face (doesn't have associated boundary conditions).
  Real computeInternalAdvectionElemMatrixContribution();

  /// Computes the matrix contribution on of the advective flux on the neighbor side of current face
  /// when the face is an internal face (doesn't have associated boundary conditions).
  Real computeInternalAdvectionNeighborMatrixContribution();

  /// The Rhie-Chow user object that provides us with the face velocity
  const INSFVRhieChowInterpolatorSegregated & _vel_provider;

  /// Container for the current velocity vector on the face to make sure we don't compute it
  /// multiple times for different terms/
  std::pair<Real, Real> _interp_coeffs;

  /// Container for the mass flux on the face which will be reused in the advection term's
  /// matrix and right hand side contribution
  Real _face_mass_flux;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;

  /// The interpolation method to use for the velocity
  Moose::FV::InterpMethod _velocity_interp_method;
};
