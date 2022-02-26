//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Computes the frictional state of nodes in mechanical contact using a mortar approach.
 */
class MortarFrictionalStateAux : public AuxKernel
{
public:
  static InputParameters validParams();

  MortarFrictionalStateAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// Tangent along the first direction
  const MooseArray<Real> & _tangent_one;

  /// Tangent along the second direction
  const MooseArray<Real> & _tangent_two;

  /// Normal contact pressure
  const MooseArray<Real> & _contact_pressure;

  /// Whether to use displaced mesh (required for this auxiliary kernel)
  const bool _use_displaced_mesh;

  /// Coefficient of friction corresponding to the contact interface
  /// TODO: Allow variable friction coefficient
  const Real _mu;

  /// Tolerance used to determine nodal contact states
  const Real _tolerance;
};
