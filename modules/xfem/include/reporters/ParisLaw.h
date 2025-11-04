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
 *  ParisLaw is a reporter that computes fatigue crack growth increment and number of cycles
 */
class CrackMeshCut3DUserObject;
class ParisLaw : public CrackGrowthReporterBase
{
public:
  static InputParameters validParams();
  ParisLaw(const InputParameters & parameters);

protected:
  virtual void computeGrowth(std::vector<int> & index) override;
  ///@{ Paris law parameters
  const Real _paris_law_c;
  const Real _paris_law_m;
  ///@}

  /// The name of the reporter with K_II fracture integral values
  const std::vector<Real> & _kii_vpp;

  /// Vector containing number of cycles to reach max_growth_increment postprocessor for each crack front point
  Real & _dn;

  /// growth rate reporter
  std::vector<Real> & _growth_increment;
};
