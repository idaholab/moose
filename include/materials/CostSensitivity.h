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

class CostSensitivity : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  CostSensitivity(const InputParameters & parameters);

  virtual void computeQpProperties() override;

protected:
  /// Base name of the material system
  const std::string _base_name;

  MaterialProperty<Real> & _sensitivity;
  const VariableValue & _design_density;
  const MaterialPropertyName _design_density_name;
  const MaterialProperty<Real> & _dcostdrho;
  const MaterialProperty<Real> & _cost;
};
