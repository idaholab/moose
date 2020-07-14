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

class CNSMassTimeDerivative;

declareADValidParams(CNSMassTimeDerivative);

/**
 * Kernel representing the time derivative term in the conservation of mass
 * equation, with strong form $\frac{\partial\left(\epsilon\rho_f\right)}{\partial t}$.
 */
class CNSMassTimeDerivative : public TimeDerivativeKernel
{
public:
  CNSMassTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADReal timeDerivative() override;

  const ADMaterialProperty<Real> & _drho_dt;

};
