//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InternalSideFluxBase.h"

/**
 * Base class for shallow water numerical fluxes.
 *
 * Provides common parameters like gravity and dry-state threshold.
 * Derived classes implement calcFlux/calcJacobian.
 */
class SWENumericalFluxBase : public InternalSideFluxBase
{
public:
  static InputParameters validParams();

  SWENumericalFluxBase(const InputParameters & parameters);
  virtual ~SWENumericalFluxBase() {}

protected:
  /// gravitational acceleration
  const Real _g;
  /// minimum depth to consider a cell wet
  const Real _h_eps;
};
