//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStressSmall.h"

/// Calculate a small strain perfect-plastic update
//
//  Small strain J2 perfect plasticity
//  This class could easily be updated to use a generic yield surface
//  The integration algorithm is general.
//
class PerfectPlasticityStressUpdate : public ComputeLagrangianStressSmall
{
public:
  static InputParameters validParams();
  PerfectPlasticityStressUpdate(const InputParameters & parameters);

protected:
  /// Implements the small stress update
  virtual void computeQpSmallStress();

private:
  // Making the following three methods an object would allow this
  // update to take a generic yield surface, instead of just J2
  /// The yield surface
  Real f(const RankTwoTensor & stress);
  /// The Jacobian of the yield surface
  RankTwoTensor n(const RankTwoTensor & stress);
  /// The Hessian of the yield surface
  RankFourTensor N(const RankTwoTensor & stress);

protected:
  /// The elasticity tensor
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  /// The yield stress
  const MaterialProperty<Real> & _yield_stress;
};
