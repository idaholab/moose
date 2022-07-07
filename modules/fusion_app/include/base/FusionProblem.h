//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblem.h"

/*
 * Need to create this special problem in order to handle customized transfer MultiAppConservativeTransfer
 * This transfer tries to create map between a 3D channel boundary and a 1D THM application
 * In a long term, we need to generalize MultiAppConservativeTransfer and move that to the MOOSE framework
*/
class FusionProblem : public FEProblem
{
public:
  static InputParameters validParams();

  FusionProblem(const InputParameters & parameters);

  BoundaryName & getMasterBoundaryName() { return _master_bdry_name; }
protected:
  BoundaryName _master_bdry_name;
};
