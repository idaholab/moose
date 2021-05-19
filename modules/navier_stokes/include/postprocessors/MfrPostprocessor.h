//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "SideIntegralPostprocessor.h"

#include <unordered_map>

/**
 * This postprocessor computes the volumetric flow rate through a boundary.
 */
class MfrPostprocessor : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  MfrPostprocessor(const InputParameters & parameters);
  void setMfr(const FaceInfo * fi, Real mfr, bool includes_area = true);

protected:
  virtual Real computeQpIntegral() override;
  virtual Real computeIntegral() override;

  std::unordered_map<const FaceInfo *, Real> _fi_to_mfr;
};
