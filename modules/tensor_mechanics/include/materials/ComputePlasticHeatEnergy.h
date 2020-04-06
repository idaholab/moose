//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

/**
 * ComputePlasticHeatEnergy computes stress * (plastic_strain - plastic_strain_old)
 * and, if currentlyComputingJacobian, then the derivative of this quantity wrt total strain
 */
class ComputePlasticHeatEnergy : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ComputePlasticHeatEnergy(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// optional parameter that allows multiple mechanics materials to be defined
  const std::string _base_name;

  /// plastic strain
  const MaterialProperty<RankTwoTensor> & _plastic_strain;

  /// old value of plastic strain
  const MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  /// stress
  const MaterialProperty<RankTwoTensor> & _stress;

  /// d(stress)/d(total strain)
  const MaterialProperty<RankFourTensor> & _Jacobian_mult;

  /// elasticity tensor
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

  /// computed property: stress * (plastic_strain - plastic_strain_old) / dt
  MaterialProperty<Real> & _plastic_heat;

  /// d(plastic_heat)/d(total strain)
  MaterialProperty<RankTwoTensor> & _dplastic_heat_dstrain;
};
