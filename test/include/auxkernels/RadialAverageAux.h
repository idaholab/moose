//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "RadialAverage.h"
/**
 * Auxkernel to output the averaged material value from RadialAverage
 */
class RadialAverageAux : public AuxKernel
{
public:
  static InputParameters validParams();

  RadialAverageAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
  const RadialAverage::Result & _average;
  RadialAverage::Result::const_iterator _elem_avg;
};
