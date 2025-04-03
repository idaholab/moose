//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowBoundary.h"

/**
 * Base class for boundary components connected to FlowChannel1PhaseBase components
 */
class FlowBoundary1PhaseBase : public FlowBoundary
{
public:
  static InputParameters validParams();

  FlowBoundary1PhaseBase(const InputParameters & params);

protected:
  virtual void init() override;

  /// Numerical flux user object name
  UserObjectName _numerical_flux_name;
  /// Name of boundary user object name
  const UserObjectName _boundary_uo_name;
};
