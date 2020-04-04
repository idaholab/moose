//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppCopyTransfer.h"

/**
 * Copies the refinement marker from the master to the sub-application.
 */
class LevelSetMeshRefinementTransfer : public MultiAppCopyTransfer
{
public:
  static InputParameters validParams();

  LevelSetMeshRefinementTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void execute() override;
};
