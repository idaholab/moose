//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVElementalKernel.h"

#include <array>

class MooseLinearVariableFVReal;

/**
 * Adds the axisymmetric viscous source term \f$\mu u_r / r^2\f$ that appears in the
 * vector Laplacian of cylindrical coordinates.  The contribution is only active when
 * the block uses an RZ coordinate system and the momentum component matches the radial
 * direction.
 */
class LinearFVRZViscousSource : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  LinearFVRZViscousSource(const InputParameters & params);

protected:
  Real computeMatrixContribution() override;
  Real computeRightHandSideContribution() override;

private:
  /// Dynamic viscosity functor evaluated at each element
  const Moose::Functor<Real> & _mu;
  /// Momentum component this source acts on (should equal the radial direction)
  const unsigned int _component;
  /// Index of the radial coordinate for the current mesh (0 -> x, 1 -> y, ...)
  const unsigned int _rz_radial_coord;
  /// Spatial dimension of the mesh
  const unsigned int _dim;
  /// Whether the deviatoric correction (-2/3 div u) is requested
  const bool _use_deviatoric_terms;
  /// Coordinate system of the active blocks (must be COORD_RZ)
  const Moose::CoordinateSystemType _coord_type;
  /// Precomputed factor (1 or 2) multiplying the implicit hoop term
  const Real _stress_multiplier;

  /// Cached pointers to the velocity components required to build divergence
  std::array<const MooseLinearVariableFVReal *, 3> _velocity_vars;

  /// Helper to access the velocity variable for a given direction
  const MooseLinearVariableFVReal & velocityVar(unsigned int dir) const;
};
