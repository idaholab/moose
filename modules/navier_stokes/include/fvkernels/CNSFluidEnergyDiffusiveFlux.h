//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CNSKernel.h"
#include "DerivativeMaterialInterface.h"

class CNSFluidEnergyDiffusiveFlux;

declareADValidParams(CNSFluidEnergyDiffusiveFlux);

/**
 * Kernel representing the diffusive component of the conservation of fluid energy
 * equation, with strong form $-\nabla\cdot\left(\kappa_f\nabla T_f\right)$.
 */
class CNSFluidEnergyDiffusiveFlux : public DerivativeMaterialInterface<CNSKernel>
{
public:
  CNSFluidEnergyDiffusiveFlux(const InputParameters & parameters);

protected:
  virtual ADReal weakResidual() override;

  virtual ADReal strongResidual() override;

  /// porosity
  const VariableValue & _eps;

  /// porosity gradient
  const VariableGradient & _grad_eps;

  /// fluid temperature gradient
  const ADMaterialProperty<RealVectorValue> & _grad_T_fluid;

  /// fluid temperature second gradient
  const ADMaterialProperty<RealTensorValue> & _grad_grad_T_fluid;

  /// fluid effective thermal conductivity
  const ADMaterialProperty<Real> & _kappa;

  /// derivative of fluid effective thermal conductivity with respect to pressure
  const MaterialProperty<Real> & _dkappa_dp;

  /// derivative of fluid effective thermal conductivity with respect to temperature
  const MaterialProperty<Real> & _dkappa_dT;

  /// pressure gradient
  const ADMaterialProperty<RealVectorValue> & _grad_pressure;

  using CNSKernel::_rz_coord;
};
