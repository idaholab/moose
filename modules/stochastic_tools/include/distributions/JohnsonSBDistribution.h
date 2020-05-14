//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "JohnsonSB.h"

/**
 * A deprecated wrapper class used to generate a Johnson SB distribution
 */
class JohnsonSBDistribution : public JohnsonSB
{
public:
  static InputParameters validParams();
  JohnsonSBDistribution(const InputParameters & parameters);
};
