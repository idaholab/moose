//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FluxBasedStrainIncrement.h"

/**
 * DeviatoricFluxBasedStrainIncrement computes strain increment based on flux (vacancy) gradient
 * Forest et. al. MSMSE 2015
 */
class DeviatoricFluxBasedStrainIncrement : public FluxBasedStrainIncrement
{
public:
  static InputParameters validParams();

  DeviatoricFluxBasedStrainIncrement(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const Real n;
};
