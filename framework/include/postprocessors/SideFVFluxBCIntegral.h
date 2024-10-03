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
#include "FVFluxBC.h"

/**
 * This postprocessor computes the side integral of different
 * finite volume flux boundary conditions.
 */
class SideFVFluxBCIntegral : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  SideFVFluxBCIntegral(const InputParameters & parameters);

protected:
  Real computeQpIntegral() override;
  Real computeFaceInfoIntegral(const FaceInfo * fi) override;

  const std::vector<std::string> _bc_names;
  std::vector<const FVFluxBC *> _bc_names;
};
