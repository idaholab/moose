//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectAction.h"

/**
 * Action that sets up BCs for porous flow module
 */
class PorousFlowAddBCAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  PorousFlowAddBCAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /**
   * Setup a BC corresponding to hot/cold fluid injection
   */
  void setupPorousFlowEnthalpySink();
};
