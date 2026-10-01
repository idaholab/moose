//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVMomentumPressure.h"

/**
 * Pressure-gradient kernel for porous momentum equations written in superficial velocity.
 */
class LinearPWCNSFVMomentumPressure : public LinearFVMomentumPressure
{
public:
  static InputParameters validParams();
  LinearPWCNSFVMomentumPressure(const InputParameters & params);

  Real computeRightHandSideContribution() override;

private:
  /// Porosity multiplying the pressure-gradient contribution.
  const Moose::Functor<Real> & _porosity;
};
