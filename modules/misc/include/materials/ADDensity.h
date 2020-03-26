//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"

class ADDensity : public ADMaterial
{
public:
  static InputParameters validParams();

  ADDensity(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

private:
  Moose::CoordinateSystemType _coord_system;
  std::vector<const ADVariableGradient *> _grad_disp;
  const ADVariableValue & _disp_r;

  const Real _initial_density;
  ADMaterialProperty<Real> & _density;
};
