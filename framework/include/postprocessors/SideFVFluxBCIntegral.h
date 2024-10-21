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

// Forward declaration
class FVFluxBC;

/**
 * This postprocessor computes the side integral of different
 * finite volume flux boundary conditions.
 */
class SideFVFluxBCIntegral : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();
  virtual void initialSetup() override;

  SideFVFluxBCIntegral(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
  virtual Real computeFaceInfoIntegral(const FaceInfo * fi) override;

  /// The names of the boundary conditions that we would like to integrate
  const std::vector<std::string> _bc_names;

  /// Pointers to the boundary conditions which will be integrated
  std::vector<FVFluxBC *> _bc_objects;
};
