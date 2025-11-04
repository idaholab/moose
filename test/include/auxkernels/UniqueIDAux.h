//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * UniqueID auxiliary value
 *
 */
class UniqueIDAux : public AuxKernel
{
public:
  static InputParameters validParams();

  UniqueIDAux(const InputParameters & parameters);

  virtual ~UniqueIDAux();

protected:
  virtual Real computeValue();
};
