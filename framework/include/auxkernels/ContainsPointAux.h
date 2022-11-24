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

namespace libMesh
{
class Point;
}

/**
 * Computes a binary field where the field is 1 in the elements that contain the point (we say
 * elements because the point may lie on a boundary between elements) and 0 everywhere else
 */
class ContainsPointAux : public AuxKernel
{
public:
  static InputParameters validParams();

  ContainsPointAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// The point we're checking that the elements contain
  const Point & _point;
};
