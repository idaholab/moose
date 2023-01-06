//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionNeumannBC.h"

class SelfShadowSideUserObject;

/**
 * Boundary condition of a Neumann style whose value is computed by a user-defined function
 */
class DirectionalFluxBC : public FunctionNeumannBC
{
public:
  static InputParameters validParams();

  DirectionalFluxBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual void precalculateResidual() override;

  /// Flux vector
  const RealVectorValue _direction;

  /// self shadow illumination calculation user object
  const SelfShadowSideUserObject * const _self_shadow;

  /// illumination state (bitmask)
  unsigned int _illumination;
};
