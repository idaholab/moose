//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVFluxBC.h"

class InputParameters;

/**
 * A parent class for boundary conditions for freely moving surfaces
 */
class INSFVFreeSurfaceBC : public INSFVFluxBC
{
public:
  static InputParameters validParams();
  INSFVFreeSurfaceBC(const InputParameters & params);
};
