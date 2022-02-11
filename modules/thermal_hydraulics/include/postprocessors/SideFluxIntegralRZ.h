//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideFluxIntegral.h"
#include "RZSymmetry.h"

/**
 * Integrates a diffusive flux over a boundary of a 2D RZ domain.
 */
class SideFluxIntegralRZ : public SideFluxIntegral, public RZSymmetry
{
public:
  static InputParameters validParams();

  SideFluxIntegralRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
};
