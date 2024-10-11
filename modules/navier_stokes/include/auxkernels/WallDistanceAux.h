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

/*
 *Computes the mixing length for the mixing length turbulence model.
 */
class WallDistanceAux : public AuxKernel
{
public:
  static InputParameters validParams();

  WallDistanceAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const std::vector<BoundaryName> & _wall_boundary_names;
};
