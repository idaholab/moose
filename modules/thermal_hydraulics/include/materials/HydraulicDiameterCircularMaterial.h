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
 * Computes hydraulic diameter for a circular flow channel
 */
class HydraulicDiameterCircularMaterial : public Material
{
public:
  HydraulicDiameterCircularMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  MaterialProperty<Real> & _D_h;

  const VariableValue & _area;

public:
  static InputParameters validParams();
};
