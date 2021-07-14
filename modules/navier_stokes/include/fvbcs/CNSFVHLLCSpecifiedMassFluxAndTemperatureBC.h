//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CNSFVHLLCBC.h"

class Function;

/**
 * Base class for specifying boundary advective fluxes for conservation of mass, momentum, and fluid
 * energy equations when using an HLLC discretization and when mass fluxes and temperature are
 * specified
 */
class CNSFVHLLCSpecifiedMassFluxAndTemperatureBC : public CNSFVHLLCBC
{
public:
  CNSFVHLLCSpecifiedMassFluxAndTemperatureBC(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  void preComputeWaveSpeed() override;

  const Function & _rhou_boundary;
  const Function * const _rhov_boundary;
  const Function * const _rhow_boundary;
  const Function & _temperature_boundary;
};
