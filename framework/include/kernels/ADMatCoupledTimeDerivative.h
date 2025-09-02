//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADCoupledTimeDerivative.h"

/**
 * This calculates the time derivative for a coupled variable
 **/
class ADMatCoupledTimeDerivative : public ADCoupledTimeDerivative
{
public:
  static InputParameters validParams();

  ADMatCoupledTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  const ADMaterialProperty<Real> & _mat_prop;
};
