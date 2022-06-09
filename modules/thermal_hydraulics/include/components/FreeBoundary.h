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
 * Adds the boundary terms resulting from an integration by parts of the
 * advection terms, using no external boundary data.
 *
 * Deprecated
 */
class FreeBoundary : public FlowBoundary
{
public:
  FreeBoundary(const InputParameters & parameters);

public:
  static InputParameters validParams();
};
