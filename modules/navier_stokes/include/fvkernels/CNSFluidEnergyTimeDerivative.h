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

class CNSFluidEnergyTimeDerivative;

declareADValidParams(CNSFluidEnergyTimeDerivative);

/**
 * Kernel representing the time derivative term in the conservation of fluid energy
 * equation, with strong form $\frac{\partial\left(\epsilon\rho_fE\right)}{\partial t}$.
 */
class CNSFluidEnergyTimeDerivative : public TimeDerivativeKernel
{
public:
  CNSFluidEnergyTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADReal timeDerivative() override;

  /// time derivative of total fluid energy density
  const ADMaterialProperty<Real> & _drho_et_dt;

};
