//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/// Considers cleavage plane anisotropy in the crack propagation

#pragma once

#include "ACInterface.h"

class ACInterfaceCleavageFracture : public ACInterface
{
public:
  static InputParameters validParams();

  ACInterfaceCleavageFracture(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// term with beta penalty
  Real betaNablaPsi();

  /// penalty for damage on planes not normal to the weak (favoured) cleavage
  /// plane (Clayton & Knap, 2015)
  const Real _beta_penalty;

  /// Plane normal to the weak cleavage plane: M in (Clayton & Knap, 2015)
  const RealVectorValue _cleavage_plane_normal;
};
