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
 * Stiffened gas fluid properties from user-specified parameters.
 */
class StiffenedGasFluidProperties : public StiffenedGasFluidPropertiesBase
{
public:
  static InputParameters validParams();

  StiffenedGasFluidProperties(const InputParameters & parameters);

protected:
  virtual void initialSetupInner() override;
};
