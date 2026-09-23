//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "PorousViscoplasticityStressUpdate.h"

/**
 * Test-only material that directly audits the compiled porous-LPS derivative and optimized kernels.
 */
class PorousViscoplasticityStressUpdateTest : public PorousViscoplasticityStressUpdate
{
public:
  static InputParameters validParams();

  PorousViscoplasticityStressUpdateTest(const InputParameters & parameters);

protected:
  using Base = PorousViscoplasticityStressUpdate;

  virtual void initQpStatefulProperties() override;

private:
  void runKernelChecks();
  void checkN1Derivatives(Real pressure, Real deffective_hydro_df) const;
  void checkHigherPowerNearZeroPressure(Real power) const;
  void checkHAndCreepKernels() const;
  void checkGaugeResidualKernels();
  void checkIndependentDerivativeKernels() const;
  void checkIndependentProjectionKernels() const;

  void checkClose(const char * label,
                  Real actual,
                  Real expected,
                  Real relative_tolerance = 5.0e-13,
                  Real absolute_tolerance = 5.0e-30) const;

  const bool _run_kernel_checks;
  bool _kernel_checks_complete = false;
};
