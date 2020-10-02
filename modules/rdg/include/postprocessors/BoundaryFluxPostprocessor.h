//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"

class BoundaryFluxBase;

/**
 * Computes the side integral of a flux entry from a BoundaryFluxBase user object
 */
class BoundaryFluxPostprocessor : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  BoundaryFluxPostprocessor(const InputParameters & parameters);

  virtual Real computeQpIntegral() override;

protected:
  /// Boundary flux user object
  const BoundaryFluxBase & _boundary_flux_uo;

  /// Index within flux vector to query
  const unsigned int & _flux_index;

  /// Did the user request to override the boundary normal?
  const bool _provided_normal;

  /// Number of components in the solution vector used to compute the flux
  const unsigned int _n_components;

  /// Variables to pass to boundary flux user object, in the correct order
  const std::vector<const VariableValue *> _U;
};
