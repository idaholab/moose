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
class ADThermalConductivityTest : public Material
{
public:
  static InputParameters validParams();

  ADThermalConductivityTest(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  ADMaterialProperty<Real> & _diffusivity;
  const ADVariableValue & _temperature;
  const ADVariableValue & _c;
};
