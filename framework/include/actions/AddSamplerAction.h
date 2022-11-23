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
 * This class adds a Sampler object.
 * The Sampler contains different sampling strategies and is used to provide
 * random values for sampled parameters using associated distributions. The
 * sampled parameters can be material properties, boundary conditions, initial
 * conditions etc.
 */
class AddSamplerAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddSamplerAction(const InputParameters & params);

  virtual void act() override;
};
