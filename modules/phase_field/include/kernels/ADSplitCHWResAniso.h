//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADSplitCHWResBase.h"

/**
 * ADSplitCHWResAniso creates the residual for the chemical potential in the split
 * form of the Cahn-Hilliard equation with a tensor (anisotropic) mobility.
 */
class ADSplitCHWResAniso : public ADSplitCHWResBase<RealTensorValue>
{
public:
  static InputParameters validParams();

  ADSplitCHWResAniso(const InputParameters & parameters);
};
