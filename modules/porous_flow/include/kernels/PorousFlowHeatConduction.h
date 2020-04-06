//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "PorousFlowDictator.h"

/**
 * Kernel = grad(test) * thermal_conductivity * grad(temperature)
 */
class PorousFlowHeatConduction : public Kernel
{
public:
  static InputParameters validParams();

  PorousFlowHeatConduction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Thermal conductivity at the quadpoints
  const MaterialProperty<RealTensorValue> & _la;

  /// d(thermal conductivity at the quadpoints)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealTensorValue>> & _dla_dvar;

  /// grad(temperature)
  const MaterialProperty<RealGradient> & _grad_t;

  /// d(gradT)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> & _dgrad_t_dvar;

  /// d(gradT)/d(grad PorousFlow variable)
  const MaterialProperty<std::vector<Real>> & _dgrad_t_dgradvar;
};
