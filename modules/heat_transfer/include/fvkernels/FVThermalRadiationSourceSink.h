//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

/**
 * Class to define the thermal radiative transfer source and sink.
 */
class FVThermalRadiationSourceSink : public FVElementalKernel
{
public:
  static InputParameters validParams();

  FVThermalRadiationSourceSink(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

protected:
  /// The coupled functor applying the radiation temperature
  const Moose::Functor<ADReal> & _temperature_radiation;

  /// The opacity field
  const Moose::Functor<ADReal> & _opacity;
};
