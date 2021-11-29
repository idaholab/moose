//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "DiracKernel.h"

class ExampleDirac : public DiracKernel
{
public:
  ExampleDirac(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void addPoints() override;
  virtual Real computeQpResidual() override;

protected:
  const Real _value;
  const Point _point;
};
