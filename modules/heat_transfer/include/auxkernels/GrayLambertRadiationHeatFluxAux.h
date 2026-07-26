//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class GrayLambertSurfaceRadiationBase;

/**
 * Radiation heat flux from a GrayLambertSurfaceRadiationBase object.
 */
class GrayLambertRadiationHeatFluxAux : public AuxKernel
{
public:
  static InputParameters validParams();

  GrayLambertRadiationHeatFluxAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Surface radiation user object containing heat flux information
  const GrayLambertSurfaceRadiationBase & _glsr_uo;
};
