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

class ADConvectiveHeatFluxTest : public Material
{
public:
  ADConvectiveHeatFluxTest(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual void computeQpProperties();

private:
  const ADVariableValue & _temperature;
  ADMaterialProperty<Real> & _t_inf;
  ADMaterialProperty<Real> & _htc;
};
