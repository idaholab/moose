//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TESTTWOPHASEFLUIDPROPERTIES_H
#define TESTTWOPHASEFLUIDPROPERTIES_H

#include "TwoPhaseFluidProperties.h"

class TestTwoPhaseFluidProperties;

template <>
InputParameters validParams<TestTwoPhaseFluidProperties>();

/**
 * Test 2-phase fluid properties
 *
 * This uses arbitrary functions for the two-phase interfaces.
 */
class TestTwoPhaseFluidProperties : public TwoPhaseFluidProperties
{
public:
  TestTwoPhaseFluidProperties(const InputParameters & parameters);

  virtual Real p_critical() const override;
  virtual Real T_sat(Real p) const override;
  virtual Real p_sat(Real T) const override;
  virtual Real dT_sat_dp(Real p) const override;
  virtual bool supportsPhaseChange() const override;
};

#endif /* TESTTWOPHASEFLUIDPROPERTIES_H */
