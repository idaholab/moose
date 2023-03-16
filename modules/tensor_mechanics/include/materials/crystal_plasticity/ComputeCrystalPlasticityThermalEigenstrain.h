//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeCrystalPlasticityEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeCrystalPlasticityThermalEigenstrain computes an eigenstrain for thermal expansion
 * with a constant thermal expansion coefficient.
 * The formulations for thermal deformation gradient and thermal eigenstrain are as follows:
 *    \dot{Fth} * Fth^{-1} = \dot{_temperature} * _thermal_expansion_coefficients;
 *    thermal_eigenstrain = 0.5 * (Fth^{T} * Fth - I)
 */
class ComputeCrystalPlasticityThermalEigenstrain
  : public DerivativeMaterialInterface<ComputeCrystalPlasticityEigenstrainBase>
{
public:
  static InputParameters validParams();

  ComputeCrystalPlasticityThermalEigenstrain(const InputParameters & parameters);

  /// We need to set initial values for lattice thermal expansion coefficients
  virtual void initQpStatefulProperties() override;

protected:
  ///Compute the deformation gradient due to thermal expansion
  virtual void computeQpDeformationGradient() override;

  ///Temperature variable value
  const VariableValue & _temperature;
  const VariableValue & _temperature_old;

  ///Stores the derivative of the deforamtion gradient w.r.t temperature
  MaterialProperty<RankTwoTensor> * _ddeformation_gradient_dT;

  ///The thermal expansion coefficient that is definted in the cartesian coordinate
  const RankTwoTensor _thermal_expansion_coefficients;

  ///Stores the thermal expansion coefficient w.r.t. the lattice symmetry axis
  MaterialProperty<RankTwoTensor> & _lattice_thermal_expansion_coefficients;
};
