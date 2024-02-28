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
#include "RankTwoTensorForward.h"

/**
 * ThermalFractureIntegral computes the summation of the derivative of the
 * eigenstrains with respect to temperature.
 */
class ThermalFractureIntegral : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ThermalFractureIntegral(const InputParameters & parameters);

  virtual void computeQpProperties() override;

protected:
  /// Base name of the material system
  const std::string _base_name;
  const std::vector<MaterialPropertyName> _eigenstrain_names;
  std::vector<const MaterialProperty<RankTwoTensor> *> _deigenstrain_dT;
  MaterialProperty<RankTwoTensor> & _total_deigenstrain_dT;
};
