//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Diffusion.h"
#include "Material.h"

// Forward Declarations
class HeatConductionKernel;

/**
 * Note: This class is named HeatConductionKernel instead of HeatConduction
 * to avoid a clash with the HeatConduction namespace.  It is registered
 * as HeatConduction, which means it can be used by that name in the input
 * file.
 */
class HeatConductionKernel : public Diffusion
{
public:
  static InputParameters validParams();

  HeatConductionKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  const MaterialProperty<Real> & _diffusion_coefficient;
  const MaterialProperty<Real> * const _diffusion_coefficient_dT;
};
