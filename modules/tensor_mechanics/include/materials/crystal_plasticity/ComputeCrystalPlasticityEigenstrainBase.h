//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrainBase.h"

/**
 * ComputeCrystalPlasticityEigenstrainBase is the base class for computing eigenstrain tensors in
 * crystal plasticity models
 */
class ComputeCrystalPlasticityEigenstrainBase : public ComputeEigenstrainBase
{
public:
  static InputParameters validParams();

  ComputeCrystalPlasticityEigenstrainBase(const InputParameters & parameters);

  /// Sets the value of the global variable _qp for inheriting classes
  void setQp(const unsigned int & qp);

  /// Sets the value of the _substep_dt for inheriting classes
  void setSubstepDt(const Real & substep_dt);

  /// We need to set initial values for deforamtion gradients too
  virtual void initQpStatefulProperties() override;

  const RankTwoTensor getDeformationGradientInverse() const;
  const RankTwoTensor getDeformationGradient() const;

  ///@{ Retained as empty methods to avoid a warning from Material.C in framework. These methods are unused in all inheriting classes and should not be overwritten.
  virtual void resetQpProperties() final {}
  virtual void resetProperties() final {}
  ///@}

protected:
  /// Compute the deformation gradient and store in _deformation_gradient
  virtual void computeQpDeformationGradient() = 0;

  ///Compute the eigenstrain and store in _eigenstrain
  void computeQpEigenstrain() override;

  ///Material property name for the deformation gradient tensor
  std::string _deformation_gradient_name;

  ///Stores the deformation gradient
  MaterialProperty<RankTwoTensor> & _deformation_gradient;
  const MaterialProperty<RankTwoTensor> & _deformation_gradient_old;

  /// Substepping time step value used within the inheriting crystal plasticity eigenstrain calculations
  Real _substep_dt;

  /**
   * Crystal rotation in the original, or reference, configuration as defined by
   * Euler angle arguments in the ComputeElasticityTensor classes
   */
  const MaterialProperty<RankTwoTensor> & _crysrot;
};
