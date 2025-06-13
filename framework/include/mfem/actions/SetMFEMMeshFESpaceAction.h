//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once

#include "Action.h"
#include "MFEMProblem.h"
/**
 * This class implements the action ensuring the mesh uses the same FE space as
 * the displacement for mesh updates.
 */
class SetMFEMMeshFESpaceAction : public Action
{
public:
  static InputParameters validParams();

  SetMFEMMeshFESpaceAction(const InputParameters & parameters);

  virtual void act() override;
};

#endif
