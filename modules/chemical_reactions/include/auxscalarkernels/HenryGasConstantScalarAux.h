//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxScalarKernel.h"

class HenryGasConstantScalarAux : public AuxScalarKernel
{
public:
  static InputParameters validParams();
  HenryGasConstantScalarAux(const InputParameters & parameters);
  virtual ~HenryGasConstantScalarAux() {}

protected:
  virtual Real computeValue() override;

  /// fluid temperature
  const VariableValue & _temperature;

  /// atomic radius
  const Real _radius;

  /// Enum used to select the type
  const enum class Saltlist { FLIBE, FLINAK, CUSTOM } _salt_list;

  /// Fit coefficients for the model
  Real _alpha;
  Real _beta;
  Real _gamma_0;
  Real _dgamma_dT;
  Real _KH0;
};
