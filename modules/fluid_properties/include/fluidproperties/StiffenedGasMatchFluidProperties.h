//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StiffenedGasFluidPropertiesBase.h"

/**
 * Stiffened gas fluid properties that fit another fluid at a (p,T) state.
 */
class StiffenedGasMatchFluidProperties : public StiffenedGasFluidPropertiesBase
{
public:
  static InputParameters validParams();

  StiffenedGasMatchFluidProperties(const InputParameters & parameters);

protected:
  virtual void initialSetupInner() override;

  /// Fluid properties to match
  const SinglePhaseFluidProperties & _fp;

  /// Pressure of state to match
  const Real _p;
  /// Temperature of state to match
  const Real _T;
};
