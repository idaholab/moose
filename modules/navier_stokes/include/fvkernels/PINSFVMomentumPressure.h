//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVMomentumPressure.h"

/**
 * Introduces the coupled pressure term into the Navier-Stokes porous media momentum equation.
 */
class PINSFVMomentumPressure : public INSFVMomentumPressure
{
public:
  static InputParameters validParams();
  PINSFVMomentumPressure(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the porosity
  const Moose::Functor<ADReal> & _eps;
};
