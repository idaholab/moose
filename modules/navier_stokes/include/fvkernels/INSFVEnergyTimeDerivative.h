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

class INSFVEnergyTimeDerivative : public FVFunctorTimeKernel
{
public:
  static InputParameters validParams();
  INSFVEnergyTimeDerivative(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the density
  const Moose::Functor<ADReal> & _rho;
  /// The time derivative of the specific enthalpy
  const Moose::Functor<ADReal> & _h_dot;
};
