//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 * Hourglass correction for Quad4 elements
 */
class HourglassCorrectionQuad4 : public Kernel
{
public:
  static InputParameters validParams();

  HourglassCorrectionQuad4(const InputParameters & parameters);

  Real computeQpResidual() override;

protected:

  const Real _penalty;

  const MooseVariable::DoFValue & _ux;
  const MooseVariable::DoFValue & _uy;
};
