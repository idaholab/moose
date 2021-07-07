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
 * Computes a component of the normal of elements.  This is mostly designed for 2D elements.  1D and
 * 3D elements are handled as special cases.
 */
class PorousFlowElementNormal : public AuxKernel
{
public:
  static InputParameters validParams();

  PorousFlowElementNormal(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Desired component
  const unsigned _component;
  /// For 1D elements, the value computed will be perpendicular to this vector
  const RealVectorValue _1D_perp;
  /// Value used for 3D elements
  const RealVectorValue _3D_default;
};
