//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "GrayLambertSurfaceRadiationBase.h"

/**
 * Boundary condition for radiative heat that is computed by the
 * GrayLambertSurfaceRadiationBase userobject
 */
class GrayLambertNeumannBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  GrayLambertNeumannBC(const InputParameters & parameters);

  static Real _sigma_stefan_boltzmann;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const GrayLambertSurfaceRadiationBase & _glsr_uo;
  bool _reconstruct_emission;
};
