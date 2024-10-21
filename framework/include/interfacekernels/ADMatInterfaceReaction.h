//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADInterfaceKernel.h"

/**
 * Implements a reaction to establish ReactionRate=k_f*u-k_b*v
 * at interface.
 */
class ADMatInterfaceReaction : public ADInterfaceKernel
{
public:
  static InputParameters validParams();

  ADMatInterfaceReaction(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  /// Forward reaction rate coefficient
  const ADMaterialProperty<Real> & _kf;

  /// Backward reaction rate coefficient
  const ADMaterialProperty<Real> & _kb;
};
