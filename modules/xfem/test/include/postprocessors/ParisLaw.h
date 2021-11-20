//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "CrackMeshCut3DUserObject.h"

class ParisLaw : public GeneralPostprocessor
{
public:
  ParisLaw(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() override;

protected:
  /// Cutter mesh
  CrackMeshCut3DUserObject * _cutter;
  /// Number of cycles for this growth increament
  unsigned long int _dn;
  /// Length of crack growth at the point with largest K
  const Real _max_growth_size;
  /// Paris law parameters
  const Real _paris_law_c;
  const Real _paris_law_m;
  /// Effective K for active cutter nodes
  std::vector<Real> _effective_k;
  /// Growth length for active cutter nodes
  std::vector<Real> _growth_size;
  /// Maximum effective K
  Real _max_k;
};
