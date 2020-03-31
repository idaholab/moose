//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"
#include "MaterialProperty.h"

class ADMatDiffusionTest : public ADKernel
{
public:
  static InputParameters validParams();

  ADMatDiffusionTest(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  const ADMaterialProperty<Real> & _ad_diff_from_ad_prop;
  const MaterialProperty<Real> & _regular_diff_from_regular_prop;
  const MooseEnum _prop_to_use;
};
