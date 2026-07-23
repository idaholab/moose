//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

/// Publishes a simple hypoelastic Cauchy stress and dsigma/d(dL) analytically, where
///   sigma_n+1 = sigma_n + C : sym(dL)        (linear isotropic elastic increment)
///   dsigma/d(dL) = C_sym                     (symmetric isotropic elasticity tangent)
/// with dL the spatial-velocity-gradient increment. Used to exercise
/// `ComputeLagrangianCauchyCustomStress` with the Jacobian tester, mirroring the contract
/// the approx-kinematics NEML2 model satisfies (dsigma/d(spatial_deformation_gradient_increment)).
class IsotropicCauchyStressTest : public Material
{
public:
  static InputParameters validParams();
  IsotropicCauchyStressTest(const InputParameters & parameters);

protected:
  void computeQpProperties() override;
  void initQpStatefulProperties() override;

  const Real _lambda;
  const Real _mu;

  /// Strain calc's `_deformation_gradient_increment` (= sym + skew of dL).
  const MaterialProperty<RankTwoTensor> & _dL;

  MaterialProperty<RankTwoTensor> & _sigma;
  const MaterialProperty<RankTwoTensor> & _sigma_old;
  MaterialProperty<RankFourTensor> & _dsigma_d_dL;
};
