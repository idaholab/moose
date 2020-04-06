//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceKernel.h"

/**
 * Base class for Diffusion equation terms coupling two different
 * variables across a subdomain boundary.
 */
class InterfaceDiffusionBase : public InterfaceKernel
{
public:
  static InputParameters validParams();

  InterfaceDiffusionBase(const InputParameters & parameters);

protected:
  /// diffusion coefficient
  const Real _D;

  /// neighbor diffusion coefficient
  const Real _D_neighbor;
};
