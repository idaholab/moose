//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeDerivativeKernel.h"

class CNSMomentumTimeDerivative;

declareADValidParams(CNSMomentumTimeDerivative);

/**
 * Kernel representing the time derivative term in the conservation of momentum
 * equation, with strong form $\frac{\partial\left(\epsilon\rho_f\vec{V}\right)}{\partial t}$.
 */
class CNSMomentumTimeDerivative : public TimeDerivativeKernel
{
public:
  CNSMomentumTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADReal timeDerivative() override;

  /// component of the momentum equation
  const unsigned int & _component;

  /// x-direction momentum time derivative
  const ADMaterialProperty<Real> & _drhou_dt;

  /// y-direction momentum time derivative
  const ADMaterialProperty<Real> & _drhov_dt;

  /// z-direction momentum time derivative
  const ADMaterialProperty<Real> & _drhow_dt;

};
