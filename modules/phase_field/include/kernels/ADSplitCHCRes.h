//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADSplitCHBase.h"

/**
 * The pair, ADSplitCHCRes and ADSplitCHWRes, splits the Cahn-Hilliard equation
 * by replacing chemical potential with 'w'.
 */
class ADSplitCHCRes : public ADSplitCHBase
{
public:
  static InputParameters validParams();

  ADSplitCHCRes(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  const ADMaterialProperty<Real> & _kappa;
  const ADVariableValue & _w;
};
