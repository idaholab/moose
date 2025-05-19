//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVTimeKernel.h"

class FVFunctorHeatConductionTimeDerivative : public FVTimeKernel
{
public:
  static InputParameters validParams();

  FVFunctorHeatConductionTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Specific heat material property
  const Moose::Functor<ADReal> & _specific_heat;

  /// Density material property
  const Moose::Functor<ADReal> & _density;
};
