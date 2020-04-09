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
 * Store current time, dt, and time step number in material properties.
 */
class TimeStepMaterial : public Material
{
public:
  static InputParameters validParams();

  TimeStepMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  MaterialProperty<Real> & _prop_dt;
  MaterialProperty<Real> & _prop_time;
  MaterialProperty<Real> & _prop_time_step;
};
