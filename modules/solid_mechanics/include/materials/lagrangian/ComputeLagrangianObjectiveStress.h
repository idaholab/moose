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
#include "DerivativeMaterialPropertyNameInterface.h"

/// Provide the Cauchy stress via an objective integration of a small stress
///
/// This class provides the Cauchy stress and associated Jacobian through
/// an objective integration of the small strain constitutive model.
///
/// The small strain model implements the computeQpSmallStress()
/// which must provide the _small_stress and _small_jacobian
/// properties.
///
/// This class is then responsible for completing the cauchy stress
/// update with an objective integration, providing _cauchy_stress
/// and _cauchy_jacobian properties
///
class ComputeLagrangianObjectiveStress : public ComputeLagrangianStressCauchy,
                                         public DerivativeMaterialPropertyNameInterface
{
public:
  static InputParameters validParams();
  ComputeLagrangianObjectiveStress(const InputParameters & parameters);

protected:
  /// Initialize the new (small) stress
  virtual void initQpStatefulProperties() override;

  /// Implement the objective update
  virtual void computeQpCauchyStress() override;

  /// Method to implement to provide the small stress update
  //    This method must provide the _small_stress and _small_jacobian
  virtual void computeQpSmallStress() = 0;

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

  /// Provided for material models that use the vorticity increment
  const MaterialProperty<RankTwoTensor> & _vorticity_increment;

  /// Deformation gradient
  const MaterialProperty<RankTwoTensor> & _def_grad;

  /// Deformation gradient
  const MaterialProperty<RankTwoTensor> & _def_grad_old;

  /// Types of objective integrations
  enum class ObjectiveRate
  {
    Truesdell,
    Jaumann,
    GreenNaghdi
  } _rate;

  /// Whether we need to perform polar decomposition
  const bool _polar_decomp;

  /// Current rotation, i.e. R in polar decomposition F = RU
  MaterialProperty<RankTwoTensor> * _rotation;

  /// Rotation at the begining of this step
  const MaterialProperty<RankTwoTensor> * _rotation_old;

  /// Derivative of rotation w.r.t. the deformation gradient
  MaterialProperty<RankFourTensor> * _d_rotation_d_def_grad;

  /// Current stretch, i.e. U in polar decomposition F = RU
  MaterialProperty<RankTwoTensor> * _stretch;

private:
  /// Objective update using the Truesdell rate
  RankTwoTensor objectiveUpdateTruesdell(const RankTwoTensor & dS);

  /// Objective update using the Jaumann rate
  RankTwoTensor objectiveUpdateJaumann(const RankTwoTensor & dS);

  /// Objective update using the Green-Naghdi rate
  RankTwoTensor objectiveUpdateGreenNaghdi(const RankTwoTensor & dS);

  /// Advect the stress using the provided kinematic tensor
  std::tuple<RankTwoTensor, RankFourTensor> advectStress(const RankTwoTensor & S0,
                                                         const RankTwoTensor & dQ) const;

  /// Make the tensor used to advect the stress
  RankFourTensor updateTensor(const RankTwoTensor & Q) const;

  /// Derivative of the action to advect stress with respect to the kinematic tensor
  RankFourTensor stressAdvectionDerivative(const RankTwoTensor & S) const;

  /// Compute the consistent tangent
  RankFourTensor cauchyJacobian(const RankFourTensor & Jinv, const RankFourTensor & U) const;

  /// Perform polar decomposition
  void polarDecomposition();
};
