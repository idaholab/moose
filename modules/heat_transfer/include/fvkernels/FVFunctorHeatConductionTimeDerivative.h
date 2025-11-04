//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFunctorTimeKernel.h"

/**
 * A finite volume kernel to add the time derivative term in the heat conduction equation, using
 * functors for the material properties
 */
class FVFunctorHeatConductionTimeDerivative : public FVFunctorTimeKernel
{
public:
  static InputParameters validParams();

  FVFunctorHeatConductionTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Specific heat capacity at constant pressure
  const Moose::Functor<ADReal> & _specific_heat;

  /// Density
  const Moose::Functor<ADReal> & _density;
};
