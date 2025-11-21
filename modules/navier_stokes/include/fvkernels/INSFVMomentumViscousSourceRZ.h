//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVElementalKernel.h"

/**
 * Adds the axisymmetric viscous source term (-mu * u_r / r^2) that appears in the Laplacian of the
 * radial momentum equation in cylindrical coordinates (no swirl). This contribution
 * is added to the Rhie-Chow interpolator as well.
 */
class INSFVMomentumViscousSourceRZ : public INSFVElementalKernel
{
public:
  static InputParameters validParams();
  INSFVMomentumViscousSourceRZ(const InputParameters & params);

  using INSFVElementalKernel::gatherRCData;
  void gatherRCData(const Elem &) override;

protected:
  ADReal computeSegregatedContribution() override;

private:
  /// Viscosity functor
  const Moose::Functor<ADReal> & _mu;
  /// Coordinate system on the restricted blocks
  const Moose::CoordinateSystemType _coord_system;
  /// Index of the radial coordinate in an RZ system
  const unsigned int _rz_radial_coord;
  /// Multiplier applied when the complete expansion form is requested
  const Real _expansion_multiplier;

  /// Compute the diagonal multiplier for this dof
  /// @param elem_arg The elem argument
  /// @param state The state argument
  ADReal computeCoefficient(const Moose::ElemArg & elem_arg, const Moose::StateArg & state) const;
};
