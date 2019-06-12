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

// Forward Declaration
template <ComputeStage>
class DarcyConvection;

declareADValidParams(DarcyConvection);

/**
 * Kernel which implements the convective term in the transient heat
 * conduction equation, and provides coupling with the Darcy pressure
 * equation.
 */
template <ComputeStage compute_stage>
class DarcyConvection : public ADKernelValue<compute_stage>
{
public:
  DarcyConvection(const InputParameters & parameters);

protected:
  /// ADKernelValue objects must override precomputeQpResidual
  virtual ADResidual precomputeQpResidual() override;

  /// The gradient of pressure
  const ADVariableGradient & _pressure_grad;

  /// These references will be set by the initialization list so that
  /// values can be pulled from the Material system.
  const MaterialProperty<Real> & _permeability;
  const MaterialProperty<Real> & _porosity;
  const ADMaterialProperty(Real) & _viscosity;
  const ADMaterialProperty(Real) & _density;
  const ADMaterialProperty(Real) & _specific_heat;

  usingKernelValueMembers;
};
