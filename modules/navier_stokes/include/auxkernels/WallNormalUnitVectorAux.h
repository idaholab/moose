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
 * Evaluate a functor (functor material property, function or variable) with either the cell-center
 * or quadrature point as the functor argument
 */
class WallNormalUnitVectorAux : public VectorAuxKernel
{
public:
  static InputParameters validParams();

  WallNormalUnitVectorAux(const InputParameters & parameters);

protected:
  virtual RealVectorValue computeValue() override;

  const std::vector<BoundaryName> & _wall_boundary_names;
};
