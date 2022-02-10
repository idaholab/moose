//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 * Base class for integrated boundary conditions for 1D problems in 3D space
 */
class ADOneDIntegratedBC : public ADIntegratedBC
{
public:
  ADOneDIntegratedBC(const InputParameters & parameters);

protected:
  /// Component of outward normals along 1-D direction
  const Real _normal;

public:
  static InputParameters validParams();
};
