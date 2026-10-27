//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowFluidStateMultiComponentBase.h"

/**
 * Test-only two-phase fluid state with an arbitrary number of fluid components, and hence
 * more than one independent total mass fraction. Every property is a simple, non-physical
 * function of pressure, temperature and all of the total mass fractions, so that the
 * multiple-z code paths in PorousFlowFluidState can be checked without a flash calculation.
 * The saturation and phase mass fractions are linear in these variables, so that Jacobians
 * which neglect their second derivatives are exact.
 */
class PorousFlowMultiComponentTestFluidState : public PorousFlowFluidStateMultiComponentBase
{
public:
  static InputParameters validParams();

  PorousFlowMultiComponentTestFluidState(const InputParameters & parameters);

  virtual std::string fluidStateName() const override;

  virtual void thermophysicalProperties(Real pressure,
                                        Real temperature,
                                        const std::vector<Real> & composition,
                                        unsigned int qp,
                                        std::vector<FluidStateProperties> & fsp) const override;

  virtual void thermophysicalProperties(const ADReal & pressure,
                                        const ADReal & temperature,
                                        const std::vector<ADReal> & composition,
                                        unsigned int qp,
                                        std::vector<FluidStateProperties> & fsp) const override;

  virtual Real totalMassFraction(
      Real pressure, Real temperature, Real Xnacl, Real saturation, unsigned int qp) const override;
};
