//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

/**
 * Implements a Neumann BC where D grad(u) = value * M on the boundary, where
 * value is a constant and M is a material property.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class MatNeumannBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  MatNeumannBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Value of constant on the boundary.
  const Real & _value;
  /// Value of material property on the boundary.
  const MaterialProperty<Real> & _boundary_prop;
};
