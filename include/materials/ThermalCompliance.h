//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialInterface.h"
#include "MathUtils.h"

class ThermalCompliance : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ThermalCompliance(const InputParameters & parameters);

  virtual void computeQpProperties() override;

protected:
  const VariableGradient & _grad_temperature;
  const MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _thermal_compliance;
};
