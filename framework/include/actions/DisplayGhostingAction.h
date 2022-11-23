//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

#include "libmesh/fe_base.h"

/**
 * Class to setup multiple AuxVariables and AuxKernels to display the ghosting when running in
 * parallel.
 */
class DisplayGhostingAction : public Action
{
public:
  static InputParameters validParams();

  DisplayGhostingAction(const InputParameters & params);

protected:
  virtual void act() override;

private:
  bool _display_ghosting;
  bool _include_local;
};
