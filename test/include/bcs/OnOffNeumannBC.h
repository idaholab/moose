//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NeumannBC.h"

/**
 * NeumanBC with ability to turn on and off
 */
class OnOffNeumannBC : public NeumannBC
{
public:
  static InputParameters validParams();

  OnOffNeumannBC(const InputParameters & parameters);

  virtual bool shouldApply();

protected:
};
