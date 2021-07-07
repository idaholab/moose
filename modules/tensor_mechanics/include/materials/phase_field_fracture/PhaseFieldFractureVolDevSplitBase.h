//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhaseFieldFractureBase.h"

/**
 * Base class for phase-field fracture models with a volumetric-deviatoric split.
 */
class PhaseFieldFractureVolDevSplitBase : public PhaseFieldFractureBase
{
public:
  static InputParameters validParams();

  PhaseFieldFractureVolDevSplitBase(const InputParameters & parameters);
};
