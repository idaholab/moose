//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * Publishes the face-oriented interface unit normal of a diffuse interface, grad(alpha) over its
 * regularized magnitude, which the volume-fraction compression flux consumes.
 */
class ConservativeDiffuseInterfaceGeometryFunctorMaterial final : public FunctorMaterial
{
public:
  static InputParameters validParams();

  ConservativeDiffuseInterfaceGeometryFunctorMaterial(const InputParameters & parameters);

private:
  /// The functor's own gradient at a cell or point argument
  template <typename SpaceArg>
  RealVectorValue alphaGradient(const SpaceArg & r, const Moose::StateArg & t) const;
  /// On an internal face, the average of the two cell gradients, so that the normal the
  /// compression flux uses belongs to the face and not to one of its cells
  RealVectorValue alphaGradient(const Moose::FaceArg & r, const Moose::StateArg & t) const;

  const Moose::Functor<Real> & _volume_fraction;

  /// Added to |grad(alpha)| so a vanishing gradient gives a finite normal
  const Real _delta_n;
};
