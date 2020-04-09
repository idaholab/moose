//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObjectBasePD.h"

/**
 * Base userObject class to compute the residual and diagonal Jacobian components for scalar
 * out-of-plane strain variable of generalized plane strain model based on peridynamic models
 */
class GeneralizedPlaneStrainUserObjectBasePD : public ElementUserObjectBasePD
{
public:
  static InputParameters validParams();

  GeneralizedPlaneStrainUserObjectBasePD(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

  /**
   * Function to return the computed residual
   * @return The computed residual
   */
  Real returnResidual() const;

  /**
   * Function to return the computed diagonal Jacobian
   * @return The computed Jacobian
   */
  Real returnJacobian() const;

protected:
  /// Option of strain formulation: SMALL or FINITE
  const MooseEnum _strain;

  /// Elasticity tensor
  const MaterialProperty<RankFourTensor> & _Cijkl;

  ///@{ Applied out-of-plane force parameters
  const Function & _pressure;
  const Real _factor;
  ///@}

  /// Residual parameter
  Real _residual;

  /// Jacobian parameter
  Real _jacobian;
};
