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
 * Subtracts the mesh velocity from the convection term in the Navier-Stokes momentum equation
 */
class INSADMeshConvection : public ADVectorKernelValue
{
public:
  static InputParameters validParams();

  INSADMeshConvection(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  /// The strong residual for this object, computed by material classes to eliminate computation
  /// duplication in simulations in which we have stabilization
  const ADMaterialProperty<RealVectorValue> & _convected_mesh_strong_residual;
};
