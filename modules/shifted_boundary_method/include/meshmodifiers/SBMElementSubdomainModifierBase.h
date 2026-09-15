//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementSubdomainModifier.h"
#include "SBMElementClassificationInterface.h"

/**
 * Common partial-element classification parameters for shifted boundary element modifiers.
 */
class SBMElementSubdomainModifierBase : public ElementSubdomainModifier,
                                        public SBMElementClassificationInterface
{
public:
  static InputParameters validParams();
  SBMElementSubdomainModifierBase(const InputParameters & parameters);
};
