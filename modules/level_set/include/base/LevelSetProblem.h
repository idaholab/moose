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

/**
 * Problem that defines a custom call to MultiAppTransfers to allow for
 * adaptivity to be transferred from master to sub-application.
 */
class LevelSetProblem : public FEProblem
{
public:
  static InputParameters validParams();

  LevelSetProblem(const InputParameters & parameters);
  virtual bool adaptMesh() override;
  virtual void computeMarkers() override;
};
