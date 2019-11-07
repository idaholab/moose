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

class GeneratedMesh;

/**
 * Aux kernel that tests periodic distance functions in GeneratedMesh
 */
class PeriodicDistanceAux : public AuxKernel
{
public:
  static InputParameters validParams();

  PeriodicDistanceAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// A point of interest in the domain
  Point _point;
};
