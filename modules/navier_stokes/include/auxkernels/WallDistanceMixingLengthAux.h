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
class WallDistanceMixingLengthAux : public AuxKernel
{
public:
  static InputParameters validParams();

  WallDistanceMixingLengthAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  std::vector<BoundaryName> _wall_boundary_names;

  const Real & _von_karman_const;
  const Real & _von_karman_const_0;
  const Real & _delta;
};
