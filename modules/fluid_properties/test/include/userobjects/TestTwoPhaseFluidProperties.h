//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TwoPhaseFluidProperties.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Test 2-phase fluid properties
 *
 * This uses arbitrary functions for the two-phase interfaces.
 */
class TestTwoPhaseFluidProperties : public TwoPhaseFluidProperties
{
public:
  static InputParameters validParams();

  TestTwoPhaseFluidProperties(const InputParameters & parameters);

  virtual Real p_critical() const override;
  virtual Real T_sat(Real p) const override;
  virtual Real p_sat(Real T) const override;
  virtual Real dT_sat_dp(Real p) const override;
  virtual Real sigma_from_T(Real T) const override;
  virtual Real dsigma_dT_from_T(Real T) const override;
  virtual bool supportsPhaseChange() const override;
};

#pragma GCC diagnostic pop
