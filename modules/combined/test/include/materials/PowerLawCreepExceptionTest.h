//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PowerLawCreepStressUpdate.h"

class PowerLawCreepExceptionTest : public PowerLawCreepStressUpdate
{
public:
  static InputParameters validParams();

  PowerLawCreepExceptionTest(const InputParameters & parameters);

protected:
  virtual Real computeResidual(const Real & effective_trial_stress, const Real & scalar) override;
  virtual Real computeDerivative(const Real & effective_trial_stress, const Real & scalar) override;
};
