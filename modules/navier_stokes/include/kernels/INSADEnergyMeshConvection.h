//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * This class computes the residual and Jacobian contributions for temperature advection from mesh
 * velocity in an ALE simulation
 */
class INSADEnergyMeshConvection : public ADKernelValue
{
public:
  static InputParameters validParams();

  INSADEnergyMeshConvection(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  const ADMaterialProperty<Real> & _temperature_convected_mesh_strong_residual;

  friend class INSADMomentumMeshAdvection;
};
