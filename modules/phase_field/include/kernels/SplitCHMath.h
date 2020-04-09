//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SplitCHCRes.h"

// Forward Declarations

/// The couple, SplitCHMath and SplitCHWRes, splits the CH equation by replacing chemical potential with 'w'.
class SplitCHMath : public SplitCHCRes
{
public:
  static InputParameters validParams();

  SplitCHMath(const InputParameters & parameters);

protected:
  virtual Real computeDFDC(PFFunctionType type);
};
