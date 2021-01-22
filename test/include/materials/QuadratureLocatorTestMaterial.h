//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

// Forward Declarations
class PenetrationLocator;

class QuadratureLocatorTestMaterial : public Material
{
public:
  static InputParameters validParams();

  QuadratureLocatorTestMaterial(const InputParameters & parameters);

protected:
  void computeQpProperties() override {}

  PenetrationLocator & _penetration_locator;
};
