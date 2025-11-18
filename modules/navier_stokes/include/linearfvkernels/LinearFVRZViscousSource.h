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
  const Moose::Functor<Real> & _mu;
  const unsigned int _component;
  const unsigned int _rz_radial_coord;
  const unsigned int _dim;
  const bool _use_deviatoric_terms;
  const Moose::CoordinateSystemType _coord_type;
  const Real _stress_multiplier;

  std::array<const MooseLinearVariableFVReal *, 3> _velocity_vars;

  const MooseLinearVariableFVReal & velocityVar(unsigned int dir) const;
};
