//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralizedMaxwellBase.h"

/**
 * This class is an implementation of a generalized Maxwell model
 * with constant mechanical properties. It consists of an arbitrary number
 * of Kelvin-Voigt units associated in series with a single spring, and an
 * optional dashpot. Having an optional dashpot may lead to divergence of
 * the time-stepping scheme and should be used with care.
 */
class GeneralizedMaxwellModel : public GeneralizedMaxwellBase
{
public:
  static InputParameters validParams();

  GeneralizedMaxwellModel(const InputParameters & parameters);

protected:
  virtual void computeQpViscoelasticProperties();
  virtual void computeQpViscoelasticPropertiesInv();

  /**
   * The elasticity tensor associated with the first spring. This is
   * !not! the true elasticity tensor of the material
   */
  RankFourTensor _C0;
  /// The elasticity tensor of each subsequent spring
  std::vector<RankFourTensor> _Ci;
  /// The viscosity of each dashpot
  std::vector<Real> _eta_i;

  /// The inverse of the elasticity tensor of the first spring
  RankFourTensor _S0;
  /// The inverse of each subsequent spring elasticity tensor
  std::vector<RankFourTensor> _Si;
};
