//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrackGrowthReporterBase.h"

/**
 * Computes critical crack growth increments from mixed-mode stress intensity factors.
 */
class CriticalCrackGrowth : public CrackGrowthReporterBase
{
public:
  static InputParameters validParams();
  CriticalCrackGrowth(const InputParameters & parameters);

protected:
  void computeGrowth(std::vector<int> & index) override;

  /// Critical fracture toughness
  const Real _k_critical;

  /// Crack growth increments at the crack front points
  std::vector<Real> & _growth_increment;
};
