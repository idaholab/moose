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
class HourglassCorrectionQuad4b : public Kernel
{
public:
  static InputParameters validParams();

  HourglassCorrectionQuad4b(const InputParameters & parameters);

  Real computeQpResidual() override;
  Real computeQpJacobian() override;

protected:
  const Real _penalty;
  const MooseVariable::DoFValue & _v;
  const unsigned int _component;
};
