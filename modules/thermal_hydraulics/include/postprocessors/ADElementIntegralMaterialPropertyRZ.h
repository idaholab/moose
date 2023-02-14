//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralMaterialProperty.h"
#include "RZSymmetry.h"

/**
 * Computes the volume integral of a material property for an RZ geometry.
 */
class ADElementIntegralMaterialPropertyRZ : public ADElementIntegralMaterialProperty,
                                            public RZSymmetry
{
public:
  static InputParameters validParams();

  ADElementIntegralMaterialPropertyRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
};
