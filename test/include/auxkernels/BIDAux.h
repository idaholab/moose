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

/**
 * returns the boundary ID
 */
class BIDAux : public AuxKernel
{
public:
  static InputParameters validParams();

  BIDAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
};
