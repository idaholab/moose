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
#include "libmesh/libmesh_common.h"

/**
 * ComputePlasticHeatEnergy computes stress * (plastic_strain - plastic_strain_old)
 * and, if currentlyComputingJacobian, then the derivative of this quantity wrt total strain
 */
class ComputeSimoHughesJ2PlasticHeatEnergy : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ComputeSimoHughesJ2PlasticHeatEnergy(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// optional parameter that allows multiple mechanics materials to be defined
  const std::string _base_name;

  /// name of effective plastic strain material property
  const std::string _ep_name;

  /// plastic strain
  const MaterialProperty<Real> & _ep;

  /// old value of plastic strain
  const MaterialProperty<Real> & _ep_old;

  /// deformation gradient
  const MaterialProperty<RankTwoTensor> & _F;

  /// stress
  const MaterialProperty<RankTwoTensor> & _cauchy_stress;

  /// computed property: stress * (plastic_strain - plastic_strain_old) / dt
  MaterialProperty<Real> & _plastic_heat;
  MaterialProperty<Real> & _dplastic_heat_dT;
  MaterialProperty<Real> & _dplastic_heat_dstrain;
};
