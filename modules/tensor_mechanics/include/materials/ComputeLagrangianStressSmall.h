//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStressCauchy.h"

/// Provide the Cauchy stress via an objective integration of a small stress
//
//    This class provides the Cauchy stress and associated Jacobian through
//    an objective integration of the small strain constitutive model.
//
//    The small strain model implements the computeQpSmallStress()
//    which must provide the _small_stress and _small_jacobian
//    properties.
//
//    This class is then responsible for completing the cauchy stress
//    update with an objective integration, providing _cauchy_stress
//    and _cauchy_jacobian properties
class ComputeLagrangianStressSmall : public ComputeLagrangianStressCauchy
{
public:
  static InputParameters validParams();
  ComputeLagrangianStressSmall(const InputParameters & parameters);
  virtual ~ComputeLagrangianStressSmall(){};

protected:
  /// Implement the objective update
  virtual void computeQpCauchyStress() override;

  /// Method to implement to provide the small stress update
  //    This method must provide the _small_stress and _small_jacobian
  virtual void computeQpSmallStress() = 0;

protected:
  /// The updated small stress
  MaterialProperty<RankTwoTensor> & _small_stress;

  /// We need the old value to get the increment
  const MaterialProperty<RankTwoTensor> & _small_stress_old;

  /// The updated small algorithmic tangent
  MaterialProperty<RankFourTensor> & _small_jacobian;

  /// We need the old Cauchy stress to do the objective integration
  const MaterialProperty<RankTwoTensor> & _cauchy_stress_old;

  /// Provided for material models that use the integrated strain
  const MaterialProperty<RankTwoTensor> & _mechanical_strain;

  /// Provided for material models that use the strain increment
  const MaterialProperty<RankTwoTensor> & _strain_increment;

private:
  /// Types of objective integrations
  enum class ObjectiveRate
  {
    Truesdell,
    Jaumann
  } _rate;

  /// Actually do the objective integration
  void _objectiveUpdate();

  /// Update tensor
  RankFourTensor _updateTensor(const RankTwoTensor & Q);

  /// Jacobian tensor for the Truesdell rate
  RankFourTensor _truesdellTangent(const RankTwoTensor & S);

  /// Jacobian tensor for the Jaumann rate
  RankFourTensor _jaumannTangent(const RankTwoTensor & S);
};
