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

/**
 * Empty material for use in simple applications that don't need material properties.
 */
class StatefulSpatialTest : public Material
{
public:
  static InputParameters validParams();

  StatefulSpatialTest(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void initQpStatefulProperties();

  MaterialProperty<Real> & _thermal_conductivity;
  const MaterialProperty<Real> & _thermal_conductivity_old;
};
