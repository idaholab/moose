//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVVariable.h"

class InputParameters;

/**
 * A special variable class for porosity which flags faces at which porosity jumps occur as
 * extrapolated boundary faces
 */
class PiecewiseConstantVariable : public INSFVVariable
{
public:
  PiecewiseConstantVariable(const InputParameters & params);

  static InputParameters validParams();

  bool isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                  const Elem * elem,
                                  const Moose::StateArg & time) const override;
};
