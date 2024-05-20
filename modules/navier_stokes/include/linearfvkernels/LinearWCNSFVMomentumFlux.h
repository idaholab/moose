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
#include "RhieChowMassFlux.h"
#include "LinearFVAdvectionDiffusionBC.h"

/**
 * Kernel that implements the stress tensor and advection terms for the momentum
 * equation.
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
  virtual void setupFaceData(const FaceInfo * face_info) override;

protected:
  /// Computes the matrix contribution of the advective flux on the element side of current face
  /// when the face is an internal face (doesn't have associated boundary conditions).
  Real computeInternalAdvectionElemMatrixContribution();

  /// Computes the matrix contribution of the advective flux on the neighbor side of current face
  /// when the face is an internal face (doesn't have associated boundary conditions).
  Real computeInternalAdvectionNeighborMatrixContribution();

  /// Computes the matrix contribution of the stress term on the current face
  /// when the face is an internal face (doesn't have associated boundary conditions).
  Real computeInternalStressMatrixContribution();

  /// Computes the right hand side contribution of the stress term on the current face
  /// when the face is an internal face (doesn't have associated boundary conditions).
  Real computeInternalStressRHSContribution();

  /// Computes the matrix contributions of the boundary conditions resulting from the stress tensor
  /// @param bc The boundary condition whose contributions should be used
  Real computeStressBoundaryMatrixContribution(const LinearFVAdvectionDiffusionBC * bc);

  /// Computes the right hand side contributions of the boundary conditions resulting
  /// from the stress tensor
  /// @param bc The boundary condition whose contributions should be used
  Real computeStressBoundaryRHSContribution(const LinearFVAdvectionDiffusionBC * bc);

  /// Computes the matrix contributions of the boundary conditions resulting
  /// from the advection term
  /// @param bc The boundary condition whose contributions should be used
  Real computeAdvectionBoundaryMatrixContribution(const LinearFVAdvectionDiffusionBC * bc);

  /// Computes the right hand side contributions of the boundary conditions resulting
  /// from the advection term
  /// @param bc The boundary condition whose contributions should be used
  Real computeAdvectionBoundaryRHSContribution(const LinearFVAdvectionDiffusionBC * bc);

  /// The dimension of the mesh
  const unsigned int _dim;

  /// The Rhie-Chow user object that provides us with the face velocity
  const RhieChowMassFlux & _mass_flux_provider;

  /// The functor for the dynamic viscosity
  const Moose::Functor<Real> & _mu;

  /// Switch to enable/disable nonorthogonal correction in the stress term
  const bool _use_nonorthogonal_correction;

  /// Switch to enable/disable deviatoric parts in the stress term
  const bool _use_deviatoric_terms;

  /// Container for the current advected interpolation coefficients on the face to make sure
  /// we don't compute it multiple times for different terms.
  std::pair<Real, Real> _advected_interp_coeffs;

  /// Container for the mass flux on the face which will be reused in the advection term's
  /// matrix and right hand side contribution
  Real _face_mass_flux;

  /// The cached matrix contribution
  Real _stress_matrix_contribution;

  /// The cached right hand side contribution
  Real _stress_rhs_contribution;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;

  /// Index x|y|z, this is mainly to handle the deviatoric parts correctly in
  /// in the stress term
  const unsigned int _index;

  /// Velocity in direction x
  const MooseLinearVariableFVReal * const _u_var;
  /// Velocity in direction y
  const MooseLinearVariableFVReal * const _v_var;
  /// Velocity in direction z
  const MooseLinearVariableFVReal * const _w_var;
};
