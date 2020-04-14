//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 * Adds contribution due to thermo-migration to the Cahn-Hilliard equation
 **/
class ADCHSoretMobility : public ADKernel
{
public:
  static InputParameters validParams();

  ADCHSoretMobility(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  // temperature variable
  const ADVariableValue & _T;

  // Temperature variable gradient
  const ADVariableGradient & _grad_T;

  // Mobility property name
  const ADMaterialProperty<Real> & _mobility;
};
